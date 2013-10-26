export include_dirs=$PWD/tool/include

if [ "$1" != 'clean' ]
then
echo "Generate Parser/Lexer"
java -jar $PWD/tool/antlr-3.5.1-complete.jar ./parser/JScript.g 
fi
