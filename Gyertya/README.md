
### Gyertyaszimuláció

Az Arduinoval egy LED-et csak kétféle állásba tudunk állítani: világít, vagy nem világít.  
Tegyük fel, hogy a LED-et olyan gyorsan tudjuk fel-le kapcsolgatni, hogy a szemünk nem veszi észre.  
Ekkor a LED fényerejét lehet szabályozni azzal, hogy az idő mennyi részében van bekapcsolt állásban.  
Ha az idő nagy részében világít, akkor fényesnek tűnik, ha az idő nagy részében nem világít, halványnak tűnik.  
Ezt az elvet hívjuk impulzusszélesség-modulációnak (Pulse-width modulation (PWM)).
Az Arduino esetében egy 0 és 256 közötti számmal mondjuk meg, hogy milyen erősen világítson a LED.

Egy gyertyát úgy tudunk imitálni, hogy a fényes és halvány szakaszok egymás után véletlenszerűen váltakoznak.

A projektet ki lehet próbálni akár egy, akár több LED-del is.  
Az Arduino UNO kártyának az alábbi pinjeit lehet használni (~ jel jelzi): 3, 5, 6, 9, 10, 11.

Az `analogWrite` függvény a PWM kimenetre

A program úgy működik, hogy az Arduino összes PWM-es kimenetére véletlenszámokat ír, így imitálva a gyertya pislákolását.

Eszközök:  
- Arduino UNO  
- 1-6 db LED  
- 1-6 db 1k ellenállás  

![Gyertya kapcsolás](../Pictures/Arduino_gyertya_bb.png)
