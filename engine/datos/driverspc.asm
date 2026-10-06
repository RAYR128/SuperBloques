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
!CanalEncenderLatch = $0D

;Valor temporal 16-bit
!ValorTemporalA = $0E
!ValorTemporalB = $0F

;usamos indices interleaved
;existen 8 canales, usamos x * 2 para acceder a canales
;por lo tanto $00-$0E es usado y $01-$0F respectivamente
!VolumenCanalL = $10
!VolumenCanalR = $11

;tics
!TicCountActualCanal = $20
!TicCountSiguienteCanal = $21

;16-bit: puntero del canal actual.
!PunteroDatosCanal1 = $30
!PunteroDatosCanal2 = $31

;16-bit: puntero de inicio del canal.
!PunteroComienzoCanal1 = $40
!PunteroComienzoCanal2 = $41

;16-bit: frecuencia del instrumento actual.
!FrecuenciaInstrumento1 = $50
!FrecuenciaInstrumento2 = $51

;el programa comienza en 0x0200
;vamos a utilizar 0x100-0x1FF para SRCN
;el stack del SPC700 vive en 01E0-01EF
;nota que el IPL de la consola limpia 0x00-0xEF
;asi que almacenamos las variables del programa ahi
base $0200
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
	mov !TablaInstrumento, #DatosInstrumento&$FF
	mov !TablaInstrumento+1, #DatosInstrumento>>8

	;datos de prueba
	mov !TempoMusica, #$30

	;inicializar los 8 canales
	mov x, #$0F
LoopInitCanal:
	mov a, DatosMusica+x
	mov !PunteroDatosCanal1+x, a
	mov !PunteroComienzoCanal1+x, a
	dec x : bpl LoopInitCanal

	;dp
	mov x, #$0E
LoopInitParCanal:
	mov a, #$00 : call CambiarInstrumentoCanal
	mov a, #$40 : mov !VolumenCanalL+x, a : mov !VolumenCanalR+x, a
	dec x : dec x : bpl LoopInitParCanal

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
	;FD [XX] [...]: control
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
	cmp a, #$ff : beq FinalizarComando
		cmp a, #$fc : bne NoEsComandoInstrumento
			;instrumento
			call LeerByteComandoCanal
			call CambiarInstrumentoCanal
			bra LeerComandos
NoEsComandoInstrumento:
		cmp a, #$fd : bne NoEsComandoControl
			;control
			mov a, (!PunteroDatosCanal1+x)
			inc !PunteroDatosCanal1+x : bne NoIncrementarHbLeerByteControl
				inc !PunteroDatosCanal2+x
NoIncrementarHbLeerByteControl:
			mov x, a
			call LlamarComandoControl
			bra LeerComandos
NoEsComandoControl:
		;antes de reproducir una siguiente nota deberiamos apagar la nota actual.
		mov $f2, #$5c
		mov $f3, !CanalActualBflag

		;silencio
		cmp a, #$fe : beq FinalizarComando
			;diferencia entre percusion/nota
			cmp a, #$f0 : bcc EsNota
				;percusion
				and a, #$0f : call CambiarInstrumentoCanal
				mov a, #$c4 ;forzar nota especifica
EsNota:
				;TO-DO: calculacion de frecuencia
				;nota
				or (!CanalEncenderLatch), (!CanalActualBflag)
FinalizarComando:
	mov a, !TicCountSiguienteCanal+x
	mov !TicCountActualCanal+x, a
NoEjecutarSiguienteComando:
	lsr !CanalActualBflag : dec x : dec x : bpl IteracionCanal

	;Fin de iteraciones, encender canales pendientes.
	mov $f2, #$5c
	mov $f3, #$00
	mov $f2, #$4c
	mov $f3, !CanalEncenderLatch
	mov !CanalEncenderLatch, #$00
ret

;entrada
;X = numero de canal
;A = numero de instrumento
CambiarInstrumentoCanal:
	mov y, #$06
	mul ya
	addw ya, !TablaInstrumento
	movw !ValorTemporalA, ya

	;deberiamos tener el registro DSP correcto para el instrumento actual.
	;esto es $x4, donde x es canal ($04, $14, $24, $34...)
	mov a, x
	xcn a
	lsr a
	or a, #$04
	mov $f2, a

	;(!ValorTemporalA) ahora contiene la tabla del instrumento actual
	mov y, #$00
RellenarValoresDSP:
	;$x4 = sample
	;$x5 = adsr 1
	;$x6 = adsr 2
	;$x7 = adsr 3
	mov a, (!ValorTemporalA)+y
	mov $f3, a : inc $f2
	inc y : cmp y, #$04 : bcc RellenarValoresDSP

	;ahora leemos los bytes de frecuencia.
	mov a, (!ValorTemporalA)+y
	mov !FrecuenciaInstrumento1+x, a
	inc y
	mov a, (!ValorTemporalA)+y
	mov !FrecuenciaInstrumento2+x, a
ret

;entrada
;X = comando
;nota que X efectivamente destruye en esta funcion y tiene que ser restaurado por cada comando especial
LlamarComandoControl:
jmp (TablaComandoControl+x)

;tabla de comandos
TablaComandoControl:
	dw Comando0VolumenCanal

;-------------------------------comandos-------------------------------
Comando0VolumenCanal:
	mov x, !CanalActual
ret

;datos musica
;-------------------------------TO-DO: cuando se añada codigo para hacer uploads actual hay que reemplazar esto!!!------------------------------
;estos son datos de ejemplo.
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

;formato
;sample, adsr 1/2/3, frecuencia (LE)
DatosInstrumento:
	db $00,$8f,$e0,$7f : dw $0300

DatosTestCanal1:
	db $20,$C4
	db $20,$C4
	db $20,$C4
	db $00
DatosTestCanalVacio:
	db $60,$FF
	db $00