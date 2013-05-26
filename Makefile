all:
	lt-comp lr apertium-hun-eng.hun.dix hun-eng.automorf.bin
	lt-comp lr apertium-hun-eng.hun-eng.dix hun-eng.autobil.bin
	lrx-comp  apertium-hun-eng.hun-eng.lrx hun-eng.autolex.bin
	apertium-preprocess-transfer apertium-hun-eng.hun-eng.t1x hun-eng.t1x.bin
	apertium-preprocess-transfer apertium-hun-eng.hun-eng.t2x hun-eng.t2x.bin
	apertium-preprocess-transfer apertium-hun-eng.hun-eng.t3x hun-eng.t3x.bin
	apertium-gen-modes modes.xml
	cp *.mode modes/
	lt-comp rl apertium-hun-eng.eng.dix hun-eng.autogen.bin
