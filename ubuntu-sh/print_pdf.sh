sudo apt-get install enscript

# Leave the output to file file.  If the file is `-', enscript sends the output to the standard output
# stdout.
enscript my_text_file.txt -o - | ps2pdf - output.pdf
