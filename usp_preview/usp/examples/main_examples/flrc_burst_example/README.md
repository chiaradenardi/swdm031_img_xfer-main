# FLRC Burst example for POC 1

## Details

- For STM32L4 only
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

For "green" LR2021 Shield:
```bash
rm -rf build/ && cmake -S examples/ -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBOARD=NUCLEO_L476 -DRAC_RADIO=lr2021 -DLEGACY_EVK_LR20XX=ON ; ninja -C build/ flrc_burst_tx flrc_burst_rx
```

For "blue" LoRa Plus LR2021 EVK:
```
rm -Rf build/; cmake -S examples/ -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBOARD=NUCLEO_L476 -DRAC_RADIO=lr2021 -DLEGACY_EVK_LR20XX=OFF -DRP_MARGIN_DELAY=10 ; ninja -C build/ flrc_burst_tx flrc_burst_rx
```
