def Message (code) :
	import sys
	MessageString = { 1 : lambda x: "Successfully created \n",
                          2 : lambda x: "Fatal: Incorrect input \n",
                          3 : lambda x: "Fatal: Overlapping objects detected \n",
                          4 : lambda x: "Fatal: Incompatible object type with neighbouring objects" }[code](str(code))
	print MessageString
	#if code > 1 : sys.exit()
	return 1

