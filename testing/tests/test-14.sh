{
	echo "should be in stdout"
	{
		cal
		{
			echo "should be in f4" > f4
			echo "but this one should be in f2" > f2
			{
				echo "more nested in f4"
				cal
			} > f4
		} > f3
		cal > f1
		cal > f3
	} > f1
	echo "should be in stdout"
}
