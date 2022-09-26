#include "nrf24.h"

#if defined(NRF24_ENABLE_DEBUG_LOG) || defined (NRF24_ENABLE_DUMP_REGISTERS)
#include <stdio.h>
#endif

#include <Arduino.h>

void nrf24_init(nrf24_t* radio, nrf24_set_pin_func set_ce_func,
        nrf24_set_pin_func set_csn_func,
        nrf24_spi_exchange_func spi_exchange_func)
{
    radio->set_ce = set_ce_func;
    radio->set_csn = set_csn_func;
    radio->spi_exchange = spi_exchange_func;

    radio->set_ce(NRF24_LOW);
    radio->set_csn(NRF24_LOW);
}

uint8_t nrf24_read_register(nrf24_t* radio, uint8_t reg)
{
    radio->set_csn(NRF24_LOW);

    radio->spi_exchange(NRF24_CMD_R_REGISTER | (reg & NRF24_R_REGISTER_MASK));
    uint8_t value = radio->spi_exchange(NRF24_CMD_NOP);

    radio->set_csn(NRF24_HIGH);

#ifdef NRF24_ENABLE_DEBUG_LOG
    Serial.printf("nrf24_read_register: %s -> ", nrf24_reg_name(reg));
    nrf24_print_reg(reg, value);
    Serial.printf("\r\n");
#endif

    return value;
}

void nrf24_write_register(nrf24_t* radio, uint8_t reg, uint8_t value)
{
#ifdef NRF24_ENABLE_DEBUG_LOG
    Serial.printf("nrf24_write_register: %s <- ", nrf24_reg_name(reg));
    nrf24_print_reg(reg, value);
    Serial.printf("\r\n");
#endif

    radio->set_csn(NRF24_LOW);

    radio->spi_exchange(NRF24_CMD_W_REGISTER | (reg & NRF24_W_REGISTER_MASK));
    radio->spi_exchange(value);

    radio->set_csn(NRF24_HIGH);
}

void nrf24_read_multi_byte_register(nrf24_t* radio, uint8_t reg, uint8_t* buf,
        uint8_t len)
{
    radio->set_csn(NRF24_LOW);

    radio->spi_exchange(NRF24_CMD_R_REGISTER | (reg & NRF24_R_REGISTER_MASK));

    for (uint8_t i = 0; i < len; ++i)
        buf[i] = radio->spi_exchange(NRF24_CMD_NOP);

#ifdef NRF24_ENABLE_DEBUG_LOG
    Serial.printf("nrf24_read_multi_byte_register: %s -> %d byte(s):", nrf24_reg_name(reg), len);
    for (uint8_t i = 0; i < len; ++i) {
        Serial.printf(" %02Xh", buf[i]);
    }
    Serial.printf("\r\n");
#endif

    radio->set_csn(NRF24_HIGH);
}

void nrf24_write_multi_byte_register(nrf24_t* radio, uint8_t reg,
        const uint8_t* buf, uint8_t len)
{
#ifdef NRF24_ENABLE_DEBUG_LOG
    Serial.printf("nrf24_write_multi_byte_register: %s <- %d byte(s):", nrf24_reg_name(reg), len);
    for (uint8_t i = 0; i < len; ++i) {
        Serial.printf(" %02Xh", buf[i]);
    }
    Serial.printf("\r\n");
#endif

    radio->set_csn(NRF24_LOW);

    radio->spi_exchange(NRF24_CMD_W_REGISTER | (reg & NRF24_W_REGISTER_MASK));

    for (uint8_t i = 0; i < len; ++i)
        radio->spi_exchange(buf[i]);

    radio->set_csn(NRF24_HIGH);
}

void nrf24_send_command(nrf24_t* radio, uint8_t command)
{
    radio->set_csn(NRF24_LOW);
    radio->spi_exchange(command);
    radio->set_csn(NRF24_HIGH);
}

void nrf24_set_rx_address(nrf24_t* radio, uint8_t pipe, const uint8_t* addr,
        uint8_t len)
{
    if (pipe > 5)
        return;

    if (pipe >= 2)
        len = 1;

    if (len > 5)
        len = 5;

//    radio->set_ce(NRF24_LOW);

    nrf24_write_multi_byte_register(radio, NRF24_REG_RX_ADDR_P0 + pipe, addr,
            len);

//    radio->set_ce(NRF24_HIGH);
}

void nrf24_get_rx_address(nrf24_t* radio, uint8_t pipe, uint8_t* addr,
        uint8_t len)
{
    if (pipe > 5)
        return;

    if (pipe >= 2)
        len = 1;

    if (len > 5)
        len = 5;

//    radio->set_ce(NRF24_LOW);

    nrf24_read_multi_byte_register(radio, NRF24_REG_RX_ADDR_P0 + pipe, addr,
            len);

//    radio->set_ce(NRF24_HIGH);
}

void nrf24_set_tx_address(nrf24_t* radio, const uint8_t* addr, uint8_t len)
{
    if (len > 5)
        len = 5;

//    radio->set_ce(NRF24_LOW);

    nrf24_write_multi_byte_register(radio, NRF24_REG_TX_ADDR, addr, len);

//    radio->set_ce(NRF24_HIGH);
}

void nrf24_get_tx_address(nrf24_t* radio, uint8_t* addr, uint8_t len)
{
    if (len > 5)
        len = 5;

//    radio->set_ce(NRF24_LOW);

    nrf24_read_multi_byte_register(radio, NRF24_REG_TX_ADDR, addr, len);

//    radio->set_ce(NRF24_HIGH);
}

void nrf24_setup_rf(nrf24_t* radio, nrf24_data_rate_t data_rate,
        nrf24_rf_out_pwr_t output_power, nrf24_lna_gain_t lna_gain)
{
    nrf24_write_register(radio,
    NRF24_REG_RF_SETUP, (data_rate << 3) | (output_power << 1) | lna_gain);
}

void nrf24_set_rf_setup(nrf24_t* radio, nrf24_rf_setup_t rf_setup)
{
    nrf24_write_register(radio, NRF24_REG_RF_SETUP, rf_setup.value);
}

nrf24_rf_setup_t nrf24_get_rf_setup(nrf24_t* radio)
{
    nrf24_rf_setup_t rf_setup;
    rf_setup.value = nrf24_read_register(radio, NRF24_REG_RF_SETUP);
    return rf_setup;
}

void nrf24_set_setup_retr(nrf24_t* radio, nrf24_setup_retr_t setup_retr)
{
    nrf24_write_register(radio, NRF24_REG_SETUP_RETR, setup_retr.value);
}

nrf24_setup_retr_t nrf24_get_setup_retr(nrf24_t* radio)
{
    nrf24_setup_retr_t setup_retr;
    setup_retr.value = nrf24_read_register(radio, NRF24_REG_SETUP_RETR);
    return setup_retr;
}

void nrf24_set_rf_channel(nrf24_t* radio, uint8_t channel)
{
    nrf24_rf_ch_t rf_ch;
    rf_ch.RF_CH = channel;
    nrf24_set_rf_ch(radio, rf_ch);
}

void nrf24_set_rf_ch(nrf24_t* radio, nrf24_rf_ch_t rf_ch)
{
    nrf24_write_register(radio, NRF24_REG_RF_CH, rf_ch.value);
}

nrf24_rf_ch_t nrf24_get_rf_ch(nrf24_t* radio)
{
    nrf24_rf_ch_t rf_ch;
    rf_ch.value = nrf24_read_register(radio, NRF24_REG_RF_CH);
    return rf_ch;
}

void nrf24_set_status(nrf24_t* radio, nrf24_status_t status)
{
    nrf24_write_register(radio, NRF24_REG_STATUS, status.value);
}

nrf24_status_t nrf24_get_status(nrf24_t* radio)
{
    nrf24_status_t status;
    status.value = nrf24_read_register(radio, NRF24_REG_STATUS);
    return status;
}

nrf24_observe_tx_t nrf24_get_observe_tx(nrf24_t* radio)
{
    nrf24_observe_tx_t observe_tx;
    observe_tx.value = nrf24_read_register(radio, NRF24_REG_OBSERVE_TX);
    return observe_tx;
}

nrf24_rpd_t nrf24_get_rpd(nrf24_t* radio)
{
    nrf24_rpd_t rpd;
    rpd.value = nrf24_read_register(radio, NRF24_REG_RPD);
    return rpd;
}

uint8_t nrf24_get_rx_payload_len(nrf24_t* radio, uint8_t pipe)
{
    if (pipe > 5)
        return 0;

    return nrf24_read_register(radio, NRF24_REG_RX_PW_P0 + pipe)
            & NRF24_RX_PW_P0_MASK;
}

void nrf24_set_rx_payload_len(nrf24_t* radio, uint8_t pipe, uint8_t len)
{
    if (pipe > 5)
        return;

    nrf24_write_register(radio, NRF24_REG_RX_PW_P0 + pipe,
            len & NRF24_RX_PW_P0_MASK);
}

nrf24_fifo_status_t nrf24_get_fifo_status(nrf24_t* radio)
{
    nrf24_fifo_status_t status;
    status.value = nrf24_read_register(radio, NRF24_REG_FIFO_STATUS);
    return status;
}

void nrf24_set_dyn_payload(nrf24_t* radio, nrf24_dynpd_t dynpd)
{
    nrf24_write_register(radio, NRF24_REG_DYNPD, dynpd.value);
}

nrf24_dynpd_t nrf24_get_dyn_payload(nrf24_t* radio)
{
    nrf24_dynpd_t dynpd;
    dynpd.value = nrf24_read_register(radio, NRF24_REG_DYNPD);
    return dynpd;
}

void nrf24_set_feature(nrf24_t* radio, nrf24_feature_t feature)
{
    nrf24_write_register(radio, NRF24_REG_FEATURE, feature.value);
}

nrf24_feature_t nrf24_get_feature(nrf24_t* radio)
{
    nrf24_feature_t feature;
    feature.value = nrf24_read_register(radio, NRF24_REG_FEATURE);
    return feature;
}

void nrf24_write_tx_payload(nrf24_t* radio, const uint8_t* buf, uint8_t len)
{
    if (len > 32)
        len = 32;

#ifdef NRF24_ENABLE_DEBUG_LOG
    Serial.printf("nrf24_write_tx_payload: %u byte(s):", len);
    for (uint8_t i = 0; i < len; ++i) {
        Serial.printf(" %02Xh", buf[i]);
    }
    Serial.printf("\r\n");
#endif

    radio->set_csn(NRF24_LOW);

    radio->spi_exchange(NRF24_CMD_W_TX_PAYLOAD);

    for (uint8_t i = 0; i < len; ++i)
        radio->spi_exchange(buf[i]);

    radio->set_csn(NRF24_HIGH);
}

void nrf24_read_rx_payload(nrf24_t* radio, uint8_t* buf, uint8_t len)
{
    if (len > 32)
        len = 32;

    radio->set_csn(NRF24_LOW);

    radio->spi_exchange(NRF24_CMD_R_RX_PAYLOAD);

    for (uint8_t i = 0; i < len; ++i)
        buf[i] = radio->spi_exchange(NRF24_CMD_NOP);

    radio->set_csn(NRF24_HIGH);

#ifdef NRF24_ENABLE_DEBUG_LOG
    Serial.printf("nrf24_read_rx_payload: %u byte(s):", len);
    for (uint8_t i = 0; i < len; ++i) {
        Serial.printf(" %02Xh", buf[i]);
    }
    Serial.printf("\r\n");
#endif
}

void nrf24_set_config(nrf24_t* radio, nrf24_config_t config)
{
    nrf24_write_register(radio, NRF24_REG_CONFIG, config.value);
}

nrf24_config_t nrf24_get_config(nrf24_t* radio)
{
    nrf24_config_t config;
    config.value = nrf24_read_register(radio, NRF24_REG_CONFIG);
    return config;
}

void nrf24_set_enable_auto_ack(nrf24_t* radio, nrf24_en_aa_t en_aa)
{
    nrf24_write_register(radio, NRF24_REG_EN_AA, en_aa.value);
}

nrf24_en_aa_t nrf24_get_enable_auto_ack(nrf24_t* radio)
{
    nrf24_en_aa_t en_aa;
    en_aa.value = nrf24_read_register(radio, NRF24_REG_EN_AA);
    return en_aa;
}

void nrf24_set_enabled_rx_addr(nrf24_t* radio, nrf24_en_rxaddr_t en_rxaddr)
{
    nrf24_write_register(radio, NRF24_REG_EN_RXADDR, en_rxaddr.value);
}

nrf24_en_rxaddr_t nrf24_get_enabled_rx_addr(nrf24_t* radio)
{
    nrf24_en_rxaddr_t en_rxaddr;
    en_rxaddr.value = nrf24_read_register(radio, NRF24_REG_EN_RXADDR);
    return en_rxaddr;
}

void nrf24_set_setup_aw(nrf24_t* radio, nrf24_setup_aw_t setup_aw)
{
    nrf24_write_register(radio, NRF24_REG_SETUP_AW, setup_aw.value);
}

nrf24_setup_aw_t nrf24_get_setup_aw(nrf24_t* radio)
{
    nrf24_setup_aw_t setup_aw;
    setup_aw.value = nrf24_read_register(radio, NRF24_REG_SETUP_AW);
    return setup_aw;
}

void nrf24_clear_interrupts(nrf24_t* radio)
{
    nrf24_status_t status = nrf24_get_status(radio);
    status.RX_DR = 1;
    status.TX_DS = 1;
    status.MAX_RT = 1;
    nrf24_set_status(radio, status);
}

void nrf24_setup_primary_receiver(nrf24_t* radio)
{
//    radio->set_ce(NRF24_LOW);

    nrf24_config_t config = nrf24_get_config(radio);
    config.PRIM_RX = 1;
    config.PWR_UP = 1;
    nrf24_set_config(radio, config);

    nrf24_set_rx_payload_len(radio, 0, NRF24_DEFAULT_PAYLOAD_LEN);

//    radio->set_ce(NRF24_HIGH);
}

void nrf24_transmit_data(nrf24_t* radio, const uint8_t* buf, uint8_t len)
{
//    radio->set_ce(NRF24_LOW);
//    nrf24_start(radio);

    nrf24_clear_interrupts(radio);

    nrf24_config_t config = nrf24_get_config(radio);
    config.PRIM_RX = 0;
    config.PWR_UP = 1;
    nrf24_set_config(radio, config);

    nrf24_flush_tx(radio);

    nrf24_write_tx_payload(radio, buf, len);
//
//
//    // FIXME
//    for (int i = 0; i < 50000; ++i)
//        asm("NOP");
//
//    nrf24_stop(radio);

//    radio->set_ce(NRF24_HIGH);
}

void nrf24_power_up_rx(nrf24_t* radio)
{
//    radio->set_ce(NRF24_LOW);

    nrf24_config_t config = nrf24_get_config(radio);
    config.PRIM_RX = 1;
    config.PWR_UP = 1;
    nrf24_set_config(radio, config);

//    radio->set_ce(NRF24_HIGH);
}

void nrf24_flush_rx(nrf24_t* radio)
{
    nrf24_send_command(radio, NRF24_CMD_FLUSH_RX);
}

void nrf24_flush_tx(nrf24_t* radio)
{
    nrf24_send_command(radio, NRF24_CMD_FLUSH_TX);
}

void nrf24_power_up(nrf24_t* radio)
{
    radio->set_ce(NRF24_HIGH);
}

void nrf24_power_down(nrf24_t* radio)
{
    radio->set_ce(NRF24_LOW);
}

#ifdef NRF24_ENABLE_DUMP_REGISTERS

void nrf24_dump_registers(nrf24_t* radio)
{
    uint8_t buf[32];

    Serial.printf("**** NRF24L01(+) Register Dump:\r\n");

    // CONFIG - Configuration Register
    nrf24_config_t config = nrf24_get_config(radio);
    Serial.printf("  0x00 - CONFIG\r\n");
    Serial.printf("    Reserved        : %d\r\n", config.reserved);
    Serial.printf("    MASK_RX_DR      : %d\r\n", config.MASK_RX_DR);
    Serial.printf("    MASK_TX_DS      : %d\r\n", config.MASK_TX_DS);
    Serial.printf("    MASK_MAX_RT     : %d\r\n", config.MASK_MAX_RT);
    Serial.printf("    EN_CRC          : %d\r\n", config.EN_CRC);
    Serial.printf("    CRCO            : %d\r\n", config.CRCO);
    Serial.printf("    PWR_UP          : %d\r\n", config.PWR_UP);
    Serial.printf("    PRIM_RX         : %d\r\n", config.PRIM_RX);

    // EN_AA - ESB: Enable 'Auto Acknowledgment'
    nrf24_en_aa_t en_aa = nrf24_get_enable_auto_ack(radio);
    Serial.printf("  0x01 - EN_AA\r\n");
    Serial.printf("    Reserved        : %d\r\n", en_aa.reserved);
    Serial.printf("    ENAA_P5         : %d\r\n", en_aa.ENAA_P5);
    Serial.printf("    ENAA_P4         : %d\r\n", en_aa.ENAA_P4);
    Serial.printf("    ENAA_P3         : %d\r\n", en_aa.ENAA_P3);
    Serial.printf("    ENAA_P2         : %d\r\n", en_aa.ENAA_P2);
    Serial.printf("    ENAA_P1         : %d\r\n", en_aa.ENAA_P1);
    Serial.printf("    ENAA_P0         : %d\r\n", en_aa.ENAA_P0);

    // EN_AA - Enable Receive Addresses
    nrf24_en_rxaddr_t en_rxaddr = nrf24_get_enabled_rx_addr(radio);
    Serial.printf("  0x02 - EN_RXADDR\r\n");
    Serial.printf("    Reserved        : %d\r\n", en_rxaddr.reserved);
    Serial.printf("    ERX_P5          : %d\r\n", en_rxaddr.EN_P5);
    Serial.printf("    ERX_P4          : %d\r\n", en_rxaddr.EN_P4);
    Serial.printf("    ERX_P3          : %d\r\n", en_rxaddr.EN_P3);
    Serial.printf("    ERX_P2          : %d\r\n", en_rxaddr.EN_P2);
    Serial.printf("    ERX_P1          : %d\r\n", en_rxaddr.EN_P1);
    Serial.printf("    ERX_P0          : %d\r\n", en_rxaddr.EN_P0);

    // SETUP_AW - Setup Address Widths
    nrf24_setup_aw_t setup_aw = nrf24_get_setup_aw(radio);
    Serial.printf("  0x03 - SETUP_AW\r\n");
    Serial.printf("    Reserved        : %d\r\n", setup_aw.reserved);
    Serial.printf("    AW              : %d\r\n", setup_aw.AW);

    // SETUP_RETR - Setup of Automatic Retransmission
    nrf24_setup_retr_t setup_retr = nrf24_get_setup_retr(radio);
    Serial.printf("  0x04 - SETUP_RETR\r\n");
    Serial.printf("    ARD             : %d\r\n", setup_retr.ARD);
    Serial.printf("    ARC             : %d\r\n", setup_retr.ARC);

    // RF_CH - RF Channel
    nrf24_rf_ch_t rf_ch = nrf24_get_rf_ch(radio);
    Serial.printf("  0x05 - RF_CH\r\n");
    Serial.printf("    Reserved        : %d\r\n", rf_ch.reserved);
    Serial.printf("    RF_CH           : %d\r\n", rf_ch.RF_CH);

    // RF_SETUP - RF Setup Register
    nrf24_rf_setup_t rf_setup = nrf24_get_rf_setup(radio);
    Serial.printf("  0x06 - RF_SETUP\r\n");
    Serial.printf("    CONT_WAVE       : %d\r\n", rf_setup.P_CONT_WAVE);
    Serial.printf("    Reserved        : %d\r\n", rf_setup.P_reserved);
    Serial.printf("    RF_DR_LOW       : %d\r\n", rf_setup.P_RF_DR_LOW);
    Serial.printf("    PLL_LOCK        : %d\r\n", rf_setup.P_PLL_LOCK);
    Serial.printf("    RF_DR_HIGH      : %d\r\n", rf_setup.P_RF_DR_HIGH);
    Serial.printf("    RF_PWR          : %d\r\n", rf_setup.P_RF_PWR);
    Serial.printf("    Obsolete (LNA)  : %d\r\n", rf_setup.P_obsolete);

    // STATUS - Status Register
    nrf24_status_t status = nrf24_get_status(radio);
    Serial.printf("  0x07 - STATUS\r\n");
    Serial.printf("    Reserved        : %d\r\n", status.reserved);
    Serial.printf("    RX_DR           : %d\r\n", status.RX_DR);
    Serial.printf("    TX_DS           : %d\r\n", status.TX_DS);
    Serial.printf("    MAX_RT          : %d\r\n", status.MAX_RT);
    Serial.printf("    RX_P_NO         : %d\r\n", status.RX_P_NO);
    Serial.printf("    TX_FULL         : %d\r\n", status.TX_FULL);

    // OBSERVE_TX - Transmit Observe Register
    nrf24_observe_tx_t observe_tx = nrf24_get_observe_tx(radio);
    Serial.printf("  0x08 - OBSERVE_TX\r\n");
    Serial.printf("    PLOS_CNT        : %d\r\n", observe_tx.PLOS_CNT);
    Serial.printf("    ARC_CNT         : %d\r\n", observe_tx.ARC_CTN);

    // RPD - Receive Power Detector Register
    nrf24_rpd_t rpd = nrf24_get_rpd(radio);
    Serial.printf("  0x09 - RPD\r\n");
    Serial.printf("    Reserved        : %d\r\n", rpd.reserved);
    Serial.printf("    RPD             : %d\r\n", rpd.RPD);

    // RX_ADDR_P0 - Receive Address Data Pipe 0
    nrf24_get_rx_address(radio, 0, buf, 5);
    Serial.printf("  0x0A - RX_ADDR_P0\r\n");
    Serial.printf("    Address         : ");
    for (int i = 0; i < 5; ++i)
        Serial.printf("%x", buf[i]);
    Serial.printf("\r\n");

    // RX_ADDR_P1 - Receive Address Data Pipe 1
    nrf24_get_rx_address(radio, 1, buf, 5);
    Serial.printf("  0x0B - RX_ADDR_P1\r\n");
    Serial.printf("    Address         : ");
    for (int i = 0; i < 5; ++i)
        Serial.printf("%x", buf[i]);
    Serial.printf("\r\n");

    // RX_ADDR_P2 - Receive Address Data Pipe 2
    nrf24_get_rx_address(radio, 2, buf, 5);
    Serial.printf("  0x0C - RX_ADDR_P3\r\n");
    Serial.printf("    Address         : %x\r\n", buf[0]);

    // RX_ADDR_P3 - Receive Address Data Pipe 3
    nrf24_get_rx_address(radio, 3, buf, 5);
    Serial.printf("  0x0D - RX_ADDR_P3\r\n");
    Serial.printf("    Address         : %x\r\n", buf[0]);

    // RX_ADDR_P4 - Receive Address Data Pipe 4
    nrf24_get_rx_address(radio, 4, buf, 5);
    Serial.printf("  0x0E - RX_ADDR_P4\r\n");
    Serial.printf("    Address         : %x\r\n", buf[0]);

    // RX_ADDR_P5 - Receive Address Data Pipe 5
    nrf24_get_rx_address(radio, 5, buf, 5);
    Serial.printf("  0x0F - RX_ADDR_P5\r\n");
    Serial.printf("    Address         : %x\r\n", buf[0]);

    // RX_ADDR_P1 - Receive Address Data Pipe 1
    nrf24_get_tx_address(radio, buf, 5);
    Serial.printf("  0x10 - TX_ADDR\r\n");
    Serial.printf("    Address         : ");
    for (int i = 0; i < 5; ++i)
        Serial.printf("%x", buf[i]);
    Serial.printf("\r\n");

    // RX_PW_P0 - Receive Payload Length Data Pipe 0
    int len = nrf24_get_rx_payload_len(radio, 0);
    Serial.printf("  0x11 - RX_PW_P0\r\n");
    Serial.printf("    Length          : %d\r\n", len);

    // RX_PW_P1 - Receive Payload Length Data Pipe 1
    len = nrf24_get_rx_payload_len(radio, 1);
    Serial.printf("  0x12 - RX_PW_P1\r\n");
    Serial.printf("    Length          : %d\r\n", len);

    // RX_PW_P2 - Receive Payload Length Data Pipe 2
    len = nrf24_get_rx_payload_len(radio, 2);
    Serial.printf("  0x13 - RX_PW_P2\r\n");
    Serial.printf("    Length          : %d\r\n", len);

    // RX_PW_P3 - Receive Payload Length Data Pipe 3
    len = nrf24_get_rx_payload_len(radio, 3);
    Serial.printf("  0x14 - RX_PW_P3\r\n");
    Serial.printf("    Length          : %d\r\n", len);

    // RX_PW_P4 - Receive Payload Length Data Pipe 4
    len = nrf24_get_rx_payload_len(radio, 4);
    Serial.printf("  0x15 - RX_PW_P4\r\n");
    Serial.printf("    Length          : %d\r\n", len);

    // RX_PW_P5 - Receive Payload Length Data Pipe 5
    len = nrf24_get_rx_payload_len(radio, 5);
    Serial.printf("  0x16 - RX_PW_P5\r\n");
    Serial.printf("    Length          : %d\r\n", len);

    // FIFO_STATUS - FIFO Status Register
    nrf24_fifo_status_t fifo_status = nrf24_get_fifo_status(radio);
    Serial.printf("  0x17 - FIFO_STATUS\r\n");
    Serial.printf("    Reserved        : %d\r\n", fifo_status.reserved2);
    Serial.printf("    TX_REUSE        : %d\r\n", fifo_status.TX_REUSE);
    Serial.printf("    TX_FULL         : %d\r\n", fifo_status.TX_FULL);
    Serial.printf("    TX_EMPTY        : %d\r\n", fifo_status.TX_EMPTY);
    Serial.printf("    Reserved        : %d\r\n", fifo_status.reserved1);
    Serial.printf("    RX_FULL         : %d\r\n", fifo_status.RX_FULL);
    Serial.printf("    RX_EMPTY        : %d\r\n", fifo_status.RX_EMPTY);

    // DYNPD - Enable Dynamic Payload Length
    nrf24_dynpd_t dynpd = nrf24_get_dyn_payload(radio);
    Serial.printf("  0x1C - DYNPD\r\n");
    Serial.printf("    Reserved        : %d\r\n", dynpd.reserved);
    Serial.printf("    DPL_P5          : %d\r\n", dynpd.DPL_P5);
    Serial.printf("    DPL_P4          : %d\r\n", dynpd.DPL_P4);
    Serial.printf("    DPL_P3          : %d\r\n", dynpd.DPL_P3);
    Serial.printf("    DPL_P2          : %d\r\n", dynpd.DPL_P2);
    Serial.printf("    DPL_P1          : %d\r\n", dynpd.DPL_P1);
    Serial.printf("    DPL_P0          : %d\r\n", dynpd.DPL_P0);

    // FEATURE - Feature Register
    nrf24_feature_t feature = nrf24_get_feature(radio);
    Serial.printf("  0x1D - FEATURE\r\n");
    Serial.printf("    Reserved        : %d\r\n", feature.reserved);
    Serial.printf("    EN_DPL          : %d\r\n", feature.EN_DPL);
    Serial.printf("    EN_ACK_PAY      : %d\r\n", feature.EN_ACK_PAY);
    Serial.printf("    EN_DYN_ACK      : %d\r\n", feature.EN_DYN_ACK);

    Serial.printf("****\r\n");
}

void nrf24_dump_status(nrf24_t* radio)
{
    nrf24_status_t status = nrf24_get_status(radio);

    Serial.printf("STATUS:");

    if (status.RX_DR)
        Serial.printf(" RX_DR");

    if (status.TX_DS)
        Serial.printf(" TX_DS");

    if (status.MAX_RT)
        Serial.printf(" MAX_RT");

    Serial.printf(" RX_P_NO=%d (", status.RX_P_NO);
    if (status.RX_P_NO <= 5)
        Serial.printf("Pipe %d)", status.RX_P_NO);
    if (status.RX_P_NO == 7)
        Serial.printf("RX FIFO empty)");

    if (status.TX_FULL)
        Serial.printf(" TX_FULL");

    Serial.printf("\r\n");
}

void nrf24_dump_fifo_status(nrf24_t* radio)
{
    nrf24_fifo_status_t fifo_status = nrf24_get_fifo_status(radio);

    Serial.printf("FIFO STATUS:");

    if (fifo_status.TX_REUSE)
        Serial.printf(" TX_REUSE");

    if (fifo_status.TX_FULL)
        Serial.printf(" TX_FULL");

    if (fifo_status.TX_EMPTY)
        Serial.printf(" TX_EMPTY");

    if (fifo_status.RX_FULL)
        Serial.printf(" RX_FULL");

    if (fifo_status.RX_EMPTY)
        Serial.printf(" RX_EMPTY");

    Serial.printf("\r\n");
}

void nrf24_dump_observe_tx(nrf24_t* radio)
{
    nrf24_observe_tx_t observe_tx = nrf24_get_observe_tx(radio);

    Serial.printf("OBSERVE TX: PLOS_CNT=%d ARC_CNT=%d\r\n",
            observe_tx.PLOS_CNT,
            observe_tx.ARC_CTN);
}

#endif

#ifdef NRF24_ENABLE_DEBUG_LOG

const char* nrf24_reg_name(uint8_t reg)
{
    switch (reg) {
    case 0x00:
        return "CONFIG";

    case 0x01:
        return "EN_AA";

    case 0x02:
        return "EN_RXADDR";

    case 0x03:
        return "SETUP_AW";

    case 0x04:
        return "SETUP_RETR";

    case 0x05:
        return "RF_CH";

    case 0x06:
        return "RF_SETUP";

    case 0x07:
        return "STATUS";

    case 0x08:
        return "OBSERVE_TX";

    case 0x09:
        return "RPD";

    case 0x0A:
        return "RX_ADDR_P0";

    case 0x0B:
        return "RX_ADDR_P1";

    case 0x0C:
        return "RX_ADDR_P2";

    case 0x0D:
        return "RX_ADDR_P3";

    case 0x0E:
        return "RX_ADDR_P4";

    case 0x0F:
        return "RX_ADDR_P5";

    case 0x10:
        return "TX_ADDR";

    case 0x11:
        return "RX_PW_P0";

    case 0x12:
        return "RX_PW_P1";

    case 0x13:
        return "RX_PW_P2";

    case 0x14:
        return "RX_PW_P3";

    case 0x15:
        return "RX_PW_P4";

    case 0x16:
        return "RX_PW_P5";

    case 0x17:
        return "FIFO_STATUS";

    case 0x1C:
        return "DYNPD";

    case 0x1D:
        return "FEATURE";

    default: {
        static char s[4] = { 0 };
        Serial.printf(s, "%02Xh", reg);
        return s;
    }
    }

    return "N/A";
}

void nrf24_print_reg(uint8_t reg, uint8_t value)
{
    switch (reg) {
    case NRF24_REG_CONFIG: {
        nrf24_config_t r;
        r.value = value;
        Serial.printf("MASK_RX_DR=%u,MASK_TX_DS=%u,MASK_MAX_RT=%u,EN_CRC=%u,CRCO=%u,PWR_UP=%u,PRIM_RX=%u",
                r.MASK_RX_DR, r.MASK_TX_DS, r.MASK_MAX_RT, r.EN_CRC, r.CRCO, r.PWR_UP, r.PRIM_RX);
        break;
    }

    case NRF24_REG_STATUS: {
        nrf24_status_t r;
        r.value = value;
        Serial.printf("RX_DR=%u,TX_DS=%u,MAX_RT=%u,RX_P_NO=%u,TX_FULL=%u",
                r.RX_DR, r.TX_DS, r.MAX_RT, r.RX_P_NO, r.TX_FULL);
        break;
    }

    case NRF24_REG_RF_CH: {
        nrf24_rf_ch_t r;
        r.value = value;
        Serial.printf("RF_CH=%u", r.RF_CH);
        break;
    }

    case NRF24_REG_FIFO_STATUS: {
        nrf24_fifo_status_t r;
        r.value = value;
        Serial.printf("TX_REUSE=%u,TX_FULL=%u,TX_EMPTY=%u,RX_FULL=%u,RX_EMPTY=%u",
                r.TX_REUSE, r.TX_FULL, r.TX_EMPTY, r.RX_FULL, r.RX_EMPTY);
        break;
    }

    default:
        Serial.printf("%02Xh", value);
        break;
    }
}

#endif
