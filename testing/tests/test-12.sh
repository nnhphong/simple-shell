{
	echo "this will not be shown in any files!" > f1
	{
		echo "this message will not be shown in f1" > f1
		echo "this one will not be shown in f1 either"
	} > f1
	echo "this message will truncate f1!" > f1
}
