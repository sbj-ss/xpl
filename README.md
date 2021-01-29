# XPL: XML Processing Language

### Prerequisites:
* libidn11-dev
* liblzma-dev (**NOT** lzma-dev)
* libonig-dev 
* libssl-dev
* libtidy-dev
* libxml2-dev
* libz-dev
* unixodbc-dev

### Compiling:
```shell
cd libxpl && make # interpreter library
cd xpl && make # command-line binary
cd xplweb && make # web server
```