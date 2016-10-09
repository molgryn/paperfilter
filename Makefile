ELECTRON_VERSION = $(shell jq .devDependencies.electron package.json -r)

ifeq ($(OS),Windows_NT)

CONFIG = build/binding.sln
all: $(CONFIG)
	node-gyp build --target=$(ELECTRON_VERSION) --arch=x64 --dist-url=https://atom.io/download/atom-shell

$(CONFIG):
	node-gyp configure --target=$(ELECTRON_VERSION) --arch=x64 --dist-url=https://atom.io/download/atom-shell

else

CONFIG = build/Makefile
all: $(CONFIG)
	HOME=~/.electron-gyp node-gyp build --target=$(ELECTRON_VERSION) --arch=x64 --dist-url=https://atom.io/download/atom-shell

$(CONFIG):
	HOME=~/.electron-gyp node-gyp configure --target=$(ELECTRON_VERSION) --arch=x64 --dist-url=https://atom.io/download/atom-shell

endif

clean:
	@node-gyp clean

fmt:
	@clang-format -i **/*.cpp **/*.hpp *.cpp *.hpp

.PHONY: all clean fmt
