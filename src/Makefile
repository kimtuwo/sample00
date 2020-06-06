# TARGET = sample
TARGET = shoot

INC_DIR	= c:/cygwin64/usr/local/lib/DxLibs
LIB_DIR = $(INC_DIR)

OPT	= -DDX_GCC_COMPILE -DDX_NON_INLINE_ASM

LIBS = -lDxLib \
	-lDxUseCLib \
	-lDxDrawFunc \
	-ljpeg \
	-lpng \
	-lzlib \
	-ltiff \
	-ltheora_static \
	-lvorbis_static \
	-lvorbisfile_static \
	-logg_static \
	-lbulletdynamics \
	-lbulletcollision \
	-lbulletmath \
	-lopusfile \
	-lopus \
	-lsilk_common \
	-lcelt \
	-liob_func

$(TARGET):$(TARGET).cpp
	g++ $(OPT) -I$(INC_DIR) -L$(LIB_DIR) $(TARGET).cpp  $(LIBS) -o  $(TARGET)
	
clean:
	rm -rf  *.exe *.o
	