Tässä toteutukseni sovellusohjelmoinnin toisesta assignmentista.
Makefile on kansiossa src/ ja siihen on toteutettu clean ja all komennot
( all ensimmäisenä tietenkin ). Serveri on nimeltään 'bank' eli se lähtee käyntiin
src/ kansiossa komennolla ./bank, ja clientteja voi sen jälkeen yhdistää palveluun
komennolla ./client. testipenkissä pienet muutokset sen arpomiin arvoihin selitetty
oppimispäiväkirjassa päivän 4.12. kohdalla, ja se käynnistyy executablella 'bench'.

Projektista jäi uupumaan master thread kokonaan, käytännössä se vain
käynnistää jonoja käsittelevän threadin, ja sitten odottaa loput threadit, kun 
jonoja käsittelevä on valmis. Toteutus on silti suht luotettava, sillä lukot estävät
dirty-readit ja dirty-writet, välillä kuitenkin clientteja jää roikkumaan benchiin.
Myös toisinaan clientit eivät yhdisty ollenkaan, esim jos testiä on ajanut monia
kertoja samalle serverin instanssille. Tällöin on auttanut esim MASTER_IN_SOCKET:n
arvon muuttaminen, ja koodin uudelleen kääntäminen. Myös ctrl+c interruptin käsittely
jäi lähes kokonaan tekemättä, mutta se ei myöskään tuntunut aivan niin oleelliselta, 
sillä tavallisilla benchin arvoilla testi on niin nopea, ettei sitä ehdi keskeyttää.
Kun taas jos testiä pidentää, tuntuu muut ongelmat estävän testaamisen.