import serial
import time
import sys

PORTA_SERIALE = 'COM3' 
BAUD_RATE = 115200

def avvia_sincronizzazione():
    try:
        print(f"Tentativo di connessione alla porta {PORTA_SERIALE}...")
        ser = serial.Serial(PORTA_SERIALE, BAUD_RATE, timeout=2)
        
        # Aspettiamo un attimo che la seriale si stabilizzi
        time.sleep(1)
        
        print("1. Connesso! Invio PING per calcolare la latenza del cavo...")
        t_inizio = time.time()
        ser.write(b"PING\n")
        
        risposta = ser.readline().decode('utf-8').strip()
        
        if risposta == "PONG":
            t_fine = time.time()
            rtt = t_fine - t_inizio
            latenza_andata = rtt / 2
            print(f"2. PONG ricevuto! Latenza stimata di sola andata: {latenza_andata*1000:.2f} ms")
            
            # Calcolo l'ora esatta aggiungendo matematicamente il tempo di viaggio
            tempo_esatto_pre_compensato = time.time() + latenza_andata
            timestamp_secondi = int(tempo_esatto_pre_compensato)
            
            print("3. Invio l'ora esatta di sistema pre-compensata...")
            comando_sync = f"SYNC {timestamp_secondi}\n"
            ser.write(comando_sync.encode('utf-8'))
            
            print(f"\n✅ Sincronizzazione completata con successo! Inviato Timestamp UNIX: {timestamp_secondi}")
            
        else:
            print(f"❌ Errore: La scheda non ha risposto correttamente. Ha risposto: '{risposta}'")
            
        ser.close()
        
    except serial.SerialException:
        print(f"❌ Errore: Impossibile aprire la porta {PORTA_SERIALE}. E' quella giusta? Il terminale Zephyr è chiuso?")
    except Exception as e:
        print(f"❌ Errore generico: {e}")

if __name__ == "__main__":
    avvia_sincronizzazione()