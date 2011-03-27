#!/usr/bin/env ruby
# Written by: Paulo H. "Taka" Torrens <paulo_torrens@hotmail.com>

SupportedTargets = ["arduino", "teensy"]
IgnoredLibs = ["usb_serial", "usb_hid", "usb_midi", "usb_disk"]
Libs = []
Headers = []
Extensions = /\.(c$|cpp$)/
class LoL
  attr_accessor :dir
  def get_files(dir = @dir)
    array = []
    Dir.foreach(dir) do |file|
      next if [".", ".."].include?(file)
      name = dir + "/" + file
      if FileTest.directory?(name)
        array += get_files(name)
      else
        if file =~ Extensions
          array.push(name)
        end
      end
    end
    return array
  end
  def command(x)
    obj = x.gsub(Extensions, ".o")
    return "#{obj}: #{x}\n\t$(CC) -c #{x} -o #{obj}"
  end
end
class Lib < LoL
  attr_accessor :name
  def initialize(name)
    @name = name + ".a"
    @dir = "lib/" + name
  end
  def to_s
    if SupportedTargets.include?(@name)
      Headers.push("-I\"#{dir}\"")
      Headers.uniq!
    end

    array = get_files
    objects = array.collect { |x| x.gsub(Extensions, ".o") }
    return [
      "#{@name}: #{objects.join(" ")}\n\t#{objects.collect { |x| "$(AR) #{@name} #{x}" }.join("\n\t")}",
      array.collect { |x| command(x) }
    ].compact.join("\n\n")
  end
  def <=> (other)
    if @name == "arduino.a" or @name == "teensy.a"
      return 1
    else
      return @name <=> other.name
    end
  end
end
class Source < LoL
  attr_accessor :objects
  def initialize
    @dir = "src"
    @array = get_files
    @objects = @array.collect { |x| x.gsub(Extensions, ".o") }
  end
  def to_s
    return @array.collect { |x| command(x) }.join("\n\n")
  end
end
Dir.foreach("lib") do |dir|
  next if [".", ".."].include?(dir)
  Libs.push(Lib.new(dir))
end
Sources = Source.new
Sources.to_s
Libs.sort!
Libs.join(" ")
Makefile = <<EOF
###################################################################
#          This file is machine-generated - Do NOT edit!          #
# Written by: Paulo H. "Taka" Torrens <paulo_torrens@hotmail.com> #
###################################################################

TARGET_BOARD = arduino
PROJECT_NAME = project
CC = avr-gcc $(CPP_FLAGS)
LD = avr-gcc $(LD_FLAGS)
AR = avr-ar $(AR_FLAGS)
OBJCP = avr-objcopy
SIZE = avr-size -A --mcu=$(MCU)
AVRDUDE = avrdude

LIBRARIES = $(TARGET_BOARD).a #{(Libs.collect { |x| x.name } - (SupportedTargets + IgnoredLibs).collect { |x| x + ".a" }).join(" ")} 
OBJECTS = #{Sources.objects.join(" ")}
INCLUDES = -I"/usr/lib/avr/include/avr" -I"./include" #{Headers.join(" ")} -I"lib/$(TARGET_BOARD)"
DEFINES = -DF_CPU=$(F_CPU)L -DARDUINO=18

TEENY_KEYBOARD_LAYOUT = LAYOUT_US_INTERNATIONAL
TEENSY_FLAGS =
TEENSY_USB = usb_serial
CPP_FLAGS = -g -Os -w -fno-exceptions -ffunction-sections -fdata-sections -mmcu=$(MCU) $(DEFINES) $(INCLUDES)
LD_FLAGS = -Os -Wl,--gc-sections -mmcu=$(MCU)
AR_FLAGS = rcs


USB=0
PORT = /dev/ttyUSB$(USB)
MCU = atmega328p
F_CPU = 16000000
FORMAT = ihex
UPLOAD_RATE = 19200
MAX_SIZE = 30720

include Makefile.local

ifeq ($(TARGET_BOARD),teensy)
  CPP_FLAGS += $(TEENSY_FLAGS) -D$(TEENY_KEYBOARD_LAYOUT)=1
  LIBRARIES += $(TEENSY_USB).a

  ifeq ($(TEENSY_USB),usb_serial)
    CPP_FLAGS += -DUSB_SERIAL
  endif
  ifeq ($(TEENSY_USB),usb_midi)
    CPP_FLAGS += -DUSB_MIDI
  endif
  ifeq ($(TEENSY_USB),usb_disk)
    CPP_FLAGS += -DUSB_DISK
  endif
  ifeq ($(TEENSY_USB),usb_hid)
    CPP_FLAGS += -DUSB_HID
  endif
endif


OUTPUT = $(PROJECT_NAME)

all: $(LIBRARIES) $(OUTPUT).hex

#{Libs.join("\n\n")}

#{Sources.to_s}

$(OUTPUT).elf: $(OBJECTS) $(LIBRARIES)
\t$(LD) $(OBJECTS) $(LIBRARIES) -lm -o $(OUTPUT).elf

$(OUTPUT).hex: $(OUTPUT).elf
\t$(OBJCP) -O ihex -R .eeprom $(OUTPUT).elf $(OUTPUT).hex
\t$(SIZE) $(OUTPUT).hex | scripts/size.rb $(MAX_SIZE) "$(OUTPUT).hex"
  
.PHONY: upload clean

upload: all
\tstty -F $(PORT) hupcl
\t$(AVRDUDE) -V -F -p $(MCU) -c stk500v1 -P $(PORT) -b $(UPLOAD_RATE) -D -Uflash:w:$(OUTPUT).hex:i 

clean:
\t@rm -f $(LIBRARIES) $(OUTPUT).elf $(OUTPUT).hex $(shell find . -follow -name "*.o")
  
Makefile: scripts/make.rb $(shell find src -follow -not -type f -newer Makefile)
\t@scripts/make.rb
EOF
File.open("Makefile", "w") do |file|
  file.write(Makefile)
end
