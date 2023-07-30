#include <array>
#include <charconv>
#include <cstring>
#include <iostream>
#include <string_view>
#include <stack>

#include <gtest/gtest.h>

using namespace std::string_view_literals;

static constexpr auto Response =
        "HTTP/1.1 200 OK\r\n"
        "Server: openresty\r\n"
        "Date: Sat, 08 Jul 2023 07:49:55 GMT\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "Content-Length: 469\r\n"
        "Connection: keep-alive\r\n"
        "X-Cache-Key: /data/2.5/weather?lat=47.5&lon=19.04\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Credentials: true\r\n"
        "Access-Control-Allow-Methods: GET, POST\r\n"
        "\r\n"
        "{\"coord\":{\"lon\":19.0404,\"lat\":47.498},\"weather\":[{\"id\":800,\"main\":\"Clear\",\"description\":\"clear sky\",\"icon\":\"01d\"}],\"base\":\"stations\",\"main\":{\"temp\":298.17,\"feels_like\":297.9,\"temp_min\":298.16,\"temp_max\":298.96,\"pressure\":1022,\"humidity\":45},\"visibility\":10000,\"wind\":{\"speed\":2.06,\"deg\":30},\"clouds\":{\"all\":0},\"dt\":1688802003,\"sys\":{\"type\":2,\"id\":2009313,\"country\":\"HU\",\"sunrise\":1688784903,\"sunset\":1688841733},\"timezone\":7200,\"id\":3054643,\"name\":\"Budapest\",\"cod\":200}";

struct HttpResponseInfo
{
    int statusCode{ 0 };
    std::string_view reasonPhrase;
    std::string_view content;
};

std::pair<std::string_view, std::string_view> parseHttpStatusLine(std::string_view line)
{
    // HTTP-Version
    const auto version = line.substr(0, line.find_first_of(' '));
    if (version != "HTTP/1.0" && version != "HTTP/1.1") {
        return {};
    }
    line.remove_prefix(version.size());

    // Space
    line.remove_prefix(line.find_first_not_of(' '));

    // Status-Code
    auto statusCode = line.substr(0, line.find_first_of(' '));
    if (statusCode.size() != 3) {
        return {};
    }
    line.remove_prefix(statusCode.size());

    // Space
    line.remove_prefix(line.find_first_not_of(' '));

    // Reason-Phrase
    auto reasonPhrase = line;

    return { statusCode, reasonPhrase };
}

std::pair<std::string_view, std::string_view> parseHttpHeaderLine(std::string_view line)
{
    // Header name
    auto name = line.substr(0, line.find_first_of(" :"));
    line.remove_prefix(name.size());

    // Separator
    line.remove_prefix(line.find_first_not_of(" :"));

    // Value
    auto value = line;

    return { name, value };
}

HttpResponseInfo parseHttpResponse(const std::string_view response)
{
    if (response.length() <= 0) {
        return {};
    }

    int maxIter{ 10000 };
    size_t pos = 0;
    static constexpr auto LineTerminator{ "\r\n"sv };
    HttpResponseInfo info;
    size_t contentLength{ 0 };
    bool statusLine{ true };

    while (maxIter--) {
        const auto eolPos = response.find(LineTerminator, pos);
        if (eolPos == std::string_view::npos) {
            return {};
        }
        const auto line = response.substr(pos, eolPos - pos);

        pos = eolPos + LineTerminator.size();

        std::cout << "pos=" << pos << ", eolPos=" << eolPos << ", line='" << line << "'\n";

        if (statusLine) {
            statusLine = false;
            const auto [statusCode, reasonPhrase] = parseHttpStatusLine(line);
            if (statusCode.size() != 3 || reasonPhrase.empty()) {
                break;
            }
            if (const auto error = std::from_chars(std::cbegin(statusCode), std::cend(statusCode), info.statusCode); error.ec != std::errc{}) {
                break;
            }
            info.reasonPhrase = reasonPhrase;
        }

        if (!line.empty()) {
            const auto [name, value] = parseHttpHeaderLine(line);

            // Content-Length
            if (name == "Content-Length") {
                if (const auto error = std::from_chars(std::cbegin(value), std::cend(value), contentLength); error.ec != std::errc{}) {
                    std::cout << "Content-Length value is invalid\n";
                    break;
                }
            }
        }

        if (line.empty() && pos < response.size()) {
            info.content = response.substr(pos);
            std::cout << "contentLength=" << info.content.size() << ", content='" << info.content << "'\n";

            if (contentLength != info.content.size()) {
                break;
            }

            return info;
        }

        if (pos >= response.size()) {
            break;
        }
    }

    return {};
}

struct HttpParserState
{
    enum class State
    {
        StatusLine,
        HeaderLine,
        Content
    } state = State::StatusLine;
    std::array<char, 1024> buffer{{}};
};

class SimpleLineBasedHttpParser
{
public:
    explicit SimpleLineBasedHttpParser(char* contentBuffer, std::size_t bufferSize)
        : _contentBuffer{ contentBuffer }
        , _bufferSize{ bufferSize }
    {}

    bool feed(const std::string& line)
    {
        if (_state == State::Content) {
            if (!bufferContent(line)) {
                _state = State::Error;
                return false;
            }

            if (_contentIndex == _contentLength) {
                _state = State::Success;
            }

            return true;
        }

        if (_state == State::Error || _state == State::Success) {
            return false;
        }

        static constexpr auto LineTerminator{ "\r\n"sv };

        const auto eolPos = line.find(LineTerminator);
        if (eolPos == std::string_view::npos) {
            return false;
        }

        const auto l = std::string_view{ line.c_str(), eolPos };

        switch (_state) {
            case State::Begin:
                if (!parseStatusLine(l)) {
                    _state = State::Error;
                    return false;
                }
                _state = State::Header;

                break;

            case State::Header:
                if (eolPos == 0 && l.empty()) {
                    if (_contentLength > 0) {
                        _state = State::Content;
                    } else {
                        _state = State::Error;
                    }
                    break;
                }

                if (!parseHeaderLine(l)) {
                    _state = State::Error;
                    return false;
                }
                break;

            case State::Content:
                if (!bufferContent(line)) {
                    _state = State::Error;
                    return false;
                }

                if (_contentIndex == _contentLength) {
                    _state = State::Success;
                }
                break;

            default:
                return false;
        }

        return true;
    }

    [[nodiscard]] int statusCode() const
    {
        return _statusCode;
    }

    [[nodiscard]] int contentLength() const
    {
        return _contentLength;
    }

    [[nodiscard]] std::string_view content() const
    {
        if (!_contentBuffer || _contentIndex == 0) {
            return {};
        }

        return std::string_view{ _contentBuffer, _contentIndex };
    }

private:
    enum class State
    {
        Begin,
        Header,
        Content,
        Success,
        Error
    };

    State _state{ State::Begin };
    int _statusCode{ 0 };
    int _contentLength{ 0 };
    char* _contentBuffer{ nullptr };
    std::size_t _bufferSize{ 0 };
    std::size_t _contentIndex{ 0 };

    bool parseStatusLine(std::string_view line)
    {
        // HTTP-Version
        const auto version = line.substr(0, line.find_first_of(' '));
        if (version != "HTTP/1.0" && version != "HTTP/1.1") {
            return false;
        }
        line.remove_prefix(version.size());

        // Space
        line.remove_prefix(line.find_first_not_of(' '));

        // Status-Code
        auto statusCode = line.substr(0, line.find_first_of(' '));
        if (statusCode.size() != 3) {
            return false;
        }
        line.remove_prefix(statusCode.size());

        // Space
        line.remove_prefix(line.find_first_not_of(' '));

#if 0
        // Reason-Phrase
        auto reasonPhrase = line;
#endif
        if (const auto error = std::from_chars(std::cbegin(statusCode), std::cend(statusCode), _statusCode); error.ec != std::errc{}) {
            return false;
        }

        return true;
    }

    bool parseHeaderLine(std::string_view line)
    {
        // Header name
        auto name = line.substr(0, line.find_first_of(" :"));
        line.remove_prefix(name.size());

        if (name.empty()) {
            return false;
        }

        // Separator
        line.remove_prefix(line.find_first_not_of(" :"));

        // Value
        auto value = line;

        if (value.empty()) {
            return false;
        }

        // Content-Length
        if (name == "Content-Length") {
            if (const auto error = std::from_chars(std::cbegin(value), std::cend(value), _contentLength); error.ec != std::errc{}) {
                std::cout << "Content-Length value is invalid\n";
                return false;
            }
        }

        return true;
    }

    bool bufferContent(const std::string_view& line)
    {
        if (line.size() > (_bufferSize - _contentIndex)) {
            return false;
        }

        std::memcpy(_contentBuffer + _contentIndex, line.data(), line.size());
        _contentIndex += line.size();

        return true;
    }
};

class ParserTest : public testing::Test
{
public:

protected:
    std::array<char, 1024> _contentBuffer{{}};
    SimpleLineBasedHttpParser _parser{ _contentBuffer.data(), _contentBuffer.size() };
};

TEST_F(ParserTest, ReadStatusLine)
{
    EXPECT_TRUE(_parser.feed("HTTP/1.1 200 OK\r\n"));
    EXPECT_EQ(_parser.statusCode(), 200);
}

TEST_F(ParserTest, ReadStatusLineAndContentLength)
{
    EXPECT_TRUE(_parser.feed("HTTP/1.1 200 OK\r\n"));
    EXPECT_EQ(_parser.statusCode(), 200);

    EXPECT_TRUE(_parser.feed("Content-Length: 469\r\n"));
    EXPECT_EQ(_parser.contentLength(), 469);
}

TEST_F(ParserTest, ReadHeaderAndContent)
{
    EXPECT_TRUE(_parser.feed("HTTP/1.1 200 OK\r\n"));
    EXPECT_EQ(_parser.statusCode(), 200);

    EXPECT_TRUE(_parser.feed("Content-Length: 13\r\n"));
    EXPECT_EQ(_parser.contentLength(), 13);

    EXPECT_TRUE(_parser.feed("\r\n\r\n"));

    EXPECT_TRUE(_parser.feed("Some content."));
    EXPECT_EQ(_parser.content().size(), 13);
    EXPECT_EQ(_parser.content(), "Some content.");
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
//    const auto response = parseHttpResponse(Response);

//    std::stack<uint8_t, std::array<uint8_t, 1024>> fixedStack;
//    for (auto i = 0; i < 1024; ++i) {
//        fixedStack.push(i);
//    }
}
