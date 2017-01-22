TEMPDIR="testscriptfolder"

rm -rf $TEMPDIR
mkdir $TEMPDIR

mv simpsh ./$TEMPDIR/

cd $TEMPDIR

touch testinput.txt

echo ""
echo "First test case: Making sure exit code is 0 for normal rdonly and wronly"
touch testoutput.txt
./simpsh --rdonly testinput.txt --wronly testoutput.txt
echo ""
if [ $? == 0 ]
then
  echo "test case 1 passed"
else
  echo "test case 1 failed"
fi

echo "Second test case: Making sure exit code is 0 for normal rdonly and wronly as well as command input"
touch testoutputerror.txt
./simpsh --rdonly testinput.txt --wronly testoutput.txt --wronly testoutputerror.txt --command 0 1 2 cat
echo ""
if [ $? == 0 ]
then
  echo "test case 2 passed"
else
  echo "test case 2 failed"
fi

echo "Third test case: non existent file"
echo ""
./simpsh --rdonly testinput.txt --wronly nothing.txt --wronly testoutputerror.txt --command 0 1 2 cat
if [ $? != 0 ]
then
  echo "test case 3 passed"
else
  echo "test case 3 failed"
fi
