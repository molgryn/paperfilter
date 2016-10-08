ELECTRON_VERSION = $(shell jq .devDependencies.electron package.json -r)
CONFIG = build/Makefile

all: $(CONFIG)
	HOME=~/.electron-gyp node-gyp build --target=$(ELECTRON_VERSION) --arch=x64 --dist-url=https://atom.io/download/atom-shell

$(CONFIG):
	HOME=~/.electron-gyp node-gyp configure --target=$(ELECTRON_VERSION) --arch=x64 --dist-url=https://atom.io/download/atom-shell

clean:
	@node-gyp clean

fmt:
	@clang-format -i **/*.cpp **/*.hpp 

.PHONY: all clean fmt
