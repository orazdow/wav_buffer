TARGET = wavbuff.exe

# comment out to use basedir
builddir = build/

SRC = main.cpp wavutil.c pa.cpp pa_ringbuffer.c rb.c 
SRCDIR_2 = lib

INCLUDE = -Iportaudio_win/include
INCLUDE += -Ilib
LIBS = -Lportaudio_win/build
LIBS += -lportaudio

CXX = g++
FLAGS = -g -Wall 

OBJECTS = $(addprefix $(builddir), $(addsuffix .o, $(basename $(notdir $(SRC)))))
# uncomment to put exe in /build 
# TARGET :=  $(addprefix $(builddir), $(TARGET))

# compile objects
$(builddir)%.o: %.cpp
	$(CXX) $(INCLUDE) $(FLAGS) -c $< -o $@ $(LIBS)
	@echo $@

$(builddir)%.o: $(SRCDIR_2)/%.c
	$(CXX) $(INCLUDE) $(FLAGS) -c $< -o $@ $(LIBS)
	@echo $@

$(builddir)%.o: $(SRCDIR_2)/%.cpp
	$(CXX) $(INCLUDE) $(FLAGS) -c $< -o $@ $(LIBS)
	@echo $@

all: make_dir $(TARGET)

# build exe
$(TARGET): $(OBJECTS)
	$(CXX) $(FLAGS) $^ -o $@ $(LIBS)

make_dir : $(builddir)

$(builddir):
	mkdir -p $(builddir)

clean:
	rm -f $(OBJECTS) $(TARGET)

print:
	@echo $(OBJECTS)
	@echo $(TARGET)

run:
	@./$(TARGET)