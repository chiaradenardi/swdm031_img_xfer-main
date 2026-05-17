# FLRC Burst example for POC 1

## Details

- For xiao nRF54L15 only (STM32L476RG porting is in progress)
- SubGig only, one channel communication
- File transfer of 20kBytes (single burst, 511 Bytes per frame)
- PHY Data Rate 2.08Mbps (US compliant), PHY Raw Data Rate 1.04Mbps (EU compliant) excluding interframe period & packet and protocol overhead
- Minimum Interframe of 200µs
- Preliminary (non LoRaWAN) LoRa WOR synchronization frame with acknowledgement before the 1st burst
- No Block ack
- No protocol security
- No protocol header
- Basic send API that will evolve based on feedback
- Single transfer direction (initiator send data)

## How to compile the application

For a device that receive data (slave):
  - Define at compilation ROLE=1 and RP_MARGIN_DELAY=10

For a device that transmit data (initiator):
  - Define at compilation ROLE=2 and RP_MARGIN_DELAY=10

Note: You can adapt parameters in `apps_configuration.h` for your application

Compilation command:

Transmitter:

``` bash
west build --pristine --board xiao_nrf54l15/nrf54l15/cpuapp --shield semtech_loraplus_expansion_board --shield semtech_wio_lr2021 usp_zephyr/samples/usp/rac/flrc_burst -- -DEXTRA_CFLAGS="-DROLE=TRANSMITTER -DRP_MARGIN_DELAY=10"
```

Receiver:

``` bash
west build --pristine --board xiao_nrf54l15/nrf54l15/cpuapp --shield semtech_loraplus_expansion_board --shield semtech_wio_lr2021 usp_zephyr/samples/usp/rac/flrc_burst -- -DEXTRA_CFLAGS="-DROLE=RECEIVER -DRP_MARGIN_DELAY=10"
```
