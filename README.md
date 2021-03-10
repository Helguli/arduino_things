# Arduino Things

My Arduino projects collected into one repository.

To upload and run code execute from terminal:

`pio run -t upload`

## Rövid leírás a projektekről

### 1. Button test

Egy kis program tesztelési céllal, hogy hogyan lehet több nyomógombot egy analóg bemenettel kezelni.

### 2. Dot matrix

Animáció egy 8x8-as LED mártixra, a kapcsolás a `Pictures` mappában megtekinthető. A tool.cpp való arra, hogy egy-egy képkocka kódját megadja.

Eszközök:  
- Arduino UNO  
- 8x8 LED mátrix

### 3. Gyertya

Az Arduino az összes PWM-es kimenetére véletlenszámokat ír, így imitálva a gyertya pislákolását.

Eszközök:  
- Arduino UNO  
- 6 db LED  
- 6 db 1k ellenállás  

### 4. LEDs

Futófény 9 db LED-del. Egy-egy nyomógombbal lehet beállítani az irányt és a világító LED-ek számát, illetve egy potenciométerrel lehet beállítani a sebességet.

Eszközök:  
- Arduino UNO  
- 9 db LED  
- 2 db nyomógomb  
- 1 db potenciométer  

### 5. MIDI-USB drums

Az Arduino USB-n keresztül küld a számítógépnek MIDI üzenetet, ha érzékeli, hogy valamelyik "dobot" megszólaltatták.

Eszközök:  
- Arduino DUE  
- 4 db piezo hangszóró (szenzorként használva, a pozitív lába az analóg bemenetre van kötve, negatív lába a földre)  
- 4 db 1M ellenállás (a szenzorokkal párhuzamosan kell kötni)  

Parancsok, amiket ki kell adni, hogy vegye a számítógép a MIDI jeleket:  
`sudo modprobe snd_seq_midi`  
`aseqdump -l`  
`fluidsynth --server --audio-driver=pulseaudio /usr/share/sounds/sf2/FluidR3_GM.sf2`  
`aconnect 20 128`

### 6. Snake

Snake játék Arduino-ra, a 8x8-as LED mátrixszal.

Eszközök:  
- Arduino UNO  
- 8x8 LED mátrix  
- Joystick  
