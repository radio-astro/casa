OUTFILE=__init__.py
echo '__all__ = ['
for file in $*
do
	pack=`basename $file .xml`
	echo "     '$pack',"
done
echo ']'
