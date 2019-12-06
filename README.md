# PunkeLele

Ey joven, te gusta la diversión sana? Convierte tu viejo controlador de Guitar Hero en un Punkelele con el que podrás tocar tus canciones de punk rock favoritas mientras se derriten los polos como una suerte de Nerón moderno. Eso sí, la Guitar hero que sea reciclada y es que el pukelele es un instrumento muy ecológico.


Como se monta.
==============

Se necesita un Teensy 3.2 con expansión de audio
https://www.pjrc.com/store/teensy3_audio.html

En el bus I2C del Teensy va uno breakout del pcf8574. Se consigue facil.  
http://www.ti.com/lit/ds/symlink/pcf8574.pdf

Los botones de la guitar hero que se conectan al pcf8574 son

1 verde
2 rojo
3 amarillo
4 azul
5 naranja
6 cuerdas abajo
7 cuardas arriba
8 select

En el directorio root de la tarjeta SD se tienen que copiar los .wavs sonidos de guitarra que se encuentran en Punkelele/Kit/guitar. Luego se sube el programa Pkl.3e al Teensy y ya está.

Como se toca.
=============
Tiene 4 modos:
  - Solo guitarra
  - Bateria + guitarra
  - Bateria + bajo + guitarra
  - Igual que el anterior (reservado para futuros usos)
  
  El botón verde y guitarrazo arriba/abajo cambia el modo
  
  El resto de los botones cambian la nota. Las afinaciones de notas no se han escogido en un orden determinado. Simplemente para ques ea cómodo tocar la mayoria de canciones punk.

Guitarrazo abajo dispara la nota, manteniendo pulsado guitarrazo arriba sighe el ritmo con la guitarra muteada.

  
  
