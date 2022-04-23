
# Assignment4
- student name:Xiaoran Xie
- student number:7884702
- how to compile all files: `make`
- how to clean all obj files: `make clean`
## main file: exfat.c
### info
- how to run: `./exfat imagename info`
- such as `./exfat volumes2/1sector-per-cluster/a4image.exfat info`
### list
- how to run: `./exfat imagename list`
- such as `./exfat volumes2/1sector-per-cluster/a4image.exfat list`
### get
- how to run: `./exfat imagename get "path/to/file.txt"`
- such as `./exfat volumes2/1sector-per-cluster/a4image.exfat get "dirs/in/dirs/in/dirs/greetings.txt"`
- Caution: If you use get command to get `README.md`, the file which you get may substitute this README.md which you read now. If you want to get `anyDirectory/README.md` or `anyDirectory/anyDirectory2/README.md`, etc. Then you can ignore this message.