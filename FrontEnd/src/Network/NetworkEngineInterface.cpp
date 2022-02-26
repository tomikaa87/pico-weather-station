#include "NetworkEngineInterface.h"

#include "NetworkEngineToFrontEnd_generated.h"

#include <iostream>
#include <cstdlib>

void NetworkEngineInterface::f()
{
    flatbuffers::FlatBufferBuilder fbb;

    const auto currentText = fbb.CreateString("current");
    const auto currentConditions = NetworkEngineToFrontEnd::CreateCurrentWeatherConditions(
        fbb, 123, 122, 124, 1000, 50, 10, 20, 100, currentText
    );

    const auto weatherData = NetworkEngineToFrontEnd::CreateWeatherData(fbb, currentConditions);

    const auto packet = NetworkEngineToFrontEnd::CreatePacket(
        fbb,
        NetworkEngineToFrontEnd::Data_WeatherData,
        weatherData.Union()
    );

    fbb.Finish(packet);

    std::vector<uint8_t> data(static_cast<size_t>(fbb.GetSize()));
    memcpy(data.data(), fbb.GetBufferPointer(), fbb.GetSize());

    auto verifier = flatbuffers::Verifier(data.data(), data.size());
    const auto verified = NetworkEngineToFrontEnd::VerifyPacketBuffer(verifier);
    const auto parsedPacket = NetworkEngineToFrontEnd::GetPacket(data.data());

    if (!verified) {
        return;
    }

    const auto wd = parsedPacket->data_as_WeatherData();
    const auto cc = wd->current();
    const auto temp = cc->temperature();
    const auto txt = cc->text()->c_str();


}