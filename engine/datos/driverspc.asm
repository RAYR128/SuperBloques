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
!TicCountActualCanal = $30
!TicCountSiguienteCanal = $31

;16-bit: puntero del canal actual.
!PunteroDatosCanal1 = $40
!PunteroDatosCanal2 = $41
!PunteroComienzoCanal1 = $50
!PunteroComienzoCanal2 = $51

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

	;resetear puertos input y correr timer 0, IPL visible
	mov $f1, #$b1
InicializarCancion:
	;-------------------------------TO-DO: cuando se añada codigo para hacer uploads actual hay que reemplazar esto!!!------------------------------
	;datos de prueba
	mov !TempoMusica, #$30

	;inicializar los 8 canales
	mov x, #$0F
LoopInitCanal:
	mov a, DatosMusica+x
	mov !PunteroDatosCanal1+x, a
	mov !PunteroComienzoCanal1+x, a
	dec x : bpl LoopInitCanal
	;------------------------------------------------------------------------------------------------------------------------------------------------
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

;Leer byte de comando para canal (compartido)
LeerByteComandoCanal:
	mov a, (!PunteroDatosCanal1+x)
	inc !PunteroDatosCanal1+x : bne NoIncrementarHbLeerByte
		inc !PunteroDatosCanal2+x
NoIncrementarHbLeerByte:
	mov y, a
ret

;ciclo actual de musica
CicloMusica:
	;$0E
	;correr codigo para todos los canales.
	mov x, #$0E
	mov !CanalActualBflag, #$80
IteracionCanal:
	mov !CanalActual, x
	dec !TicCountActualCanal+x : bne NoEjecutarSiguienteComando
LeerComandos:
	;lista de comandos
	;00: final de canal
	;01-7F: duracion de nota
	;80-EF: notas
	;F0-FB: percusion
	;FC [XX]: instrumento
	;FD [XX]: tempo
	;FE: silencio
	;FF: rest (no hace nada)
	call LeerByteComandoCanal : bmi EsComandoSpec
		bne EsComandoDuracion
			mov a, !PunteroComienzoCanal1+x
			mov !PunteroDatosCanal1+x, a
			mov a, !PunteroComienzoCanal2+x
			mov !PunteroDatosCanal2+x, a
			bra LeerComandos
EsComandoDuracion:
		mov !TicCountSiguienteCanal+x, a
		bra LeerComandos
EsComandoSpec:
	;TO-DO: implementar comando de notas
	mov a, !TicCountSiguienteCanal+x
	mov !TicCountActualCanal+x, a
NoEjecutarSiguienteComando:
	lsr !CanalActualBflag : dec x : dec x : bpl IteracionCanal
ret

;datos musica
;8 punteros (16-bit) para pointers para cada canal
DatosMusica:
	dw DatosTestCanal1
	dw DatosTestCanalVacio
	dw DatosTestCanalVacio
	dw DatosTestCanalVacio
	dw DatosTestCanalVacio
	dw DatosTestCanalVacio
	dw DatosTestCanalVacio
	dw DatosTestCanalVacio

DatosTestCanal1:
	db $7F,$F0,$00
DatosTestCanalVacio:
	db $7F,$F0,$00