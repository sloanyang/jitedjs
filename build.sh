export include_dirs=$PWD/tool/include

echo "Generate Parser/Lexer"
java -jar $PWD/tool/antlr-3.5.1-complete.jar ./parser/JSMinus.g -o ./parser

echo "Patching into Parser..."
sed -i 's/=  this->matchToken/= (ImplTraits::CommonTokenType*) this->matchToken/g' ./parser/JSMinusParser.cpp

echo "Start building..."
make $1 2>&1 >/dev/null

echo "Run ./main"
