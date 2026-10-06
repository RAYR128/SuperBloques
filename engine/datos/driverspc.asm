;driver de sonido para SuperBloques

;.0 -
;.1 -
;.2 -
;.3 -
;.4 -
;.5 -
;.6 -
;.7 = musica desactivada
!EstadoEjecucion = $04

!TablaInstrumento = $06
!CanalActual = $08
!CanalActualBflag = $09
!TempoMusica = $0A
!ContadorTick = $0B

;usamos indices interleaved
;existen 8 canales, usamos x * 2 para acceder a canales
;por lo tanto $00-$0E es usado y $01-$0F respectivamente
!VolumenCanalL = $10
!VolumenCanalR = $11
!InstrumentoCanal = $20
!NotaActualCanal = $21

;el programa comienza en 0x0100
;nota que el IPL de la consola limpia 0x00-0xEF
;asi que almacenamos las variables del programa ahi
base $0100
SpcArranque:
	clrp

	;inicializar DSP
	mov x, #20
InicializarDsp:
	mov a, ValoresDefectoDSP+x : mov $f2, a
	mov a, ValoresDefectoDSP+1+x : mov $f3, a
	dec x : dec x : bpl InicializarDsp

	;frecuencia timer 0 = 2ms
	mov $fa, #$10
;esto corre siempre
BucleSonido:
	;leer timer 0, esperar a una respuesta
	mov y, $fd : beq BucleSonido

	;aqui es donde usamos el tempo de la musica y lo multiplicamos por el timer 0
	;esto lo usamos para saber cuantos ticks avanzamos en la cancion
	;y asi determinar si tenemos que llamar a un ciclo de sonido o no dependiendo del estado del overflow.
	mov a, !TempoMusica : mul ya : clrc : adc a, !ContadorTick : mov !ContadorTick, a
	bcs CumpleTick
		mov a, y : beq BucleSonido
CumpleTick:
	bbs1 !EstadoEjecucion.7, BucleSonido
		call CicloMusica
bra BucleSonido

;valores de inicializacion del chip de sonido
ValoresDefectoDSP:
	db $0C,$7F ;vol master L = 127 (100%)
	db $1C,$7F ;vol master R = 127 (100%)
	db $2C,$00 ;vol eco L = 0 (desact.)
	db $3C,$00 ;vol eco L = 0 (desact.)
	db $0D,$60 ;feedback eco = 0 (desact.)
	db $2D,$00 ;PmodX = 0 (desact.)
	db $3D,$00 ;NoiseX = 0 (desact.)
	db $4D,$00 ;EcoEnX = 0 (desact.)
	db $6C,$20 ;FLG = Reset
	db $7D,$00 ;EDL = 0 (desact.)
	db $6D,$00 ;ESA = 0x0000

;ciclo actual de musica
CicloMusica:
ret