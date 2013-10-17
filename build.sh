export include_dirs=$PWD/tool/include

if [ $1 != 'clean' ]
then
echo "Generate Parser/Lexer"
java -jar $PWD/tool/antlr-3.5.1-complete.jar ./parser/JSMinus.g 

echo "Patching into Parser..."
sed -i 's/=  this->matchToken/= (ImplTraits::CommonTokenType*) this->matchToken/g' ./parser/JSMinusParser.cpp
fi

echo "Start building..."
make $1 

#2>&1 >/dev/null
if [ $1 != 'clean' ]
then
echo "Run ./main"
else
echo "Cleaned!"
fi

