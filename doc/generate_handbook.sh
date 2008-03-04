#/bin/sh
echo "Generating PDF in the subdirectory 'pdf'"
docbook2pdf -o pdf index.docbook

echo "Generating HTML in the subdirectory 'html'"
docbook2html --output html index.docbook 
