import sys
vout={"InitialScan":1,
	  "Scan":1,
	  "BeginGatherSearch":1,
	  "GatherSearch":1,
	  "RelativeGather":1,
	  "InitialScan2":1,
	  "Scan2":1,
	  "BeginGatherSearch2":1,
	  "GatherSearch2":1,
	  "RelativeGather2":1,
	  "InitialScan3":1,
	  "Scan3":1,
	  "BeginGatherSearch3":1,
	  "GatherSearch3":1,
	  "RelativeGather3":1,
	  "InitialScan4":1,
	  "Scan4":1,
	  "BeginGatherSearch4":1,
	  "GatherSearch4":1,
	  "RelativeGather4":1}
voutscan = {"InitialScan":1,
			"Scan":1,
			"InitialScan2":1,
			"Scan2":1,
			"InitialScan3":1,
			"Scan3":1,
			"InitialScan4":1,
			"Scan4":1
	}
voutscatter={"InitialScan":1,
			 "Scan":1,
			 "RelativeScatter":1,
			 "InitialScan2":1,
			 "Scan2":1,
			 "RelativeScatter2":1,
			 "InitialScan3":1,
			 "Scan3":1,
			 "RelativeScatter3":1,
			 "InitialScan4":1,
			 "Scan4":1,
			 "RelativeScatter4":1}
costs= {"InitialScan4":(32,0,4,{"add":1,
							 "cmp":3}),
		"Scan4":(8,0,4,{"add":1,
					 "cmp":1}),
		"BeginGatherSearch4":(24,0,4,{"cmp":4,
								   "bit":1,
								   "mul":1}),
		"GatherSearch4":(4,20,4,{"add":3,
								"cmp":3,
								"mul":1}),
		"RelativeGather4":(4,16,16,{"add":1}),
		"RelativeScatter4":(4,16,16,{"add":1}),
		"InitialScan3":(24,0,4,{"add":1,
							 "cmp":3}),
		"Scan3":(8,0,4,{"add":1,
					 "cmp":1}),
		"BeginGatherSearch3":(20,0,4,{"cmp":4,
								   "bit":1,
								   "mul":1}),
		"GatherSearch3":(4,16,4,{"add":3,
								"cmp":3,
								"mul":1}),
		"RelativeGather3":(4,12,12,{"add":1}),
		"RelativeScatter3":(4,12,12,{"add":1}),
		"InitialScan2":(16,0,4,{"add":1,
							 "cmp":3}),
		"Scan2":(8,0,4,{"add":1,
					 "cmp":1}),
		"BeginGatherSearch2":(16,0,4,{"cmp":4,
								   "bit":1,
								   "mul":1}),
		"GatherSearch2":(4,12,4,{"add":3,
								"cmp":3,
								"mul":1}),
		"RelativeGather2":(4,8,8,{"add":1}),
		"RelativeScatter2":(4,8,8,{"add":1}),
		"InitialScan":(8,0,4,{"add":1,
							 "cmp":3}),
		"Scan":(8,0,4,{"add":1,
					 "cmp":1}),
		"BeginGatherSearch":(8,0,4,{"cmp":4,
								   "bit":1,
								   "mul":1}),
		"GatherSearch":(4,8,4,{"add":3,
								"cmp":3,
								"mul":1}),
		"RelativeGather":(4,4,4,{"add":1}),
		"RelativeScatter":(4,4,4,{"add":1}),		
		"processSlice":(8,0,0,{"cmp":10,
 								"add":7,
	 						    "push":1,
								"mul":1}),
		"processTriangles":(4,48,36,{"div":1,
									  "push":3,
									  "mul":1,
									  "add":12,
									  "cmp":1}),
		"processFirstTriangles":(28,12,12,{"div":1,
										   "add":3}),
		"processTrianglesNoCompactOneOut":(6.4,12,12,{"div":1,
													 "add":3}),
		"produceTriP":(48,0,8,{"cmp":3,
								"bit":2}),
		"computeNeighbors":(8,192,144,{"mul":12*12+12*6,
									   "add":12*6+12*3,
									   "cmp":12*3+12,
									   "bit":18}),
		"smallEnough":(192,0,192,{"add":102,
								  "mul":12,
								  "cmp":21,
								  "bit":9}),
		"splitTriangles":(8,192,96,{"add":60,
									"mul":63,
									"cmp":15,
									"bit":15,
									"div":6,
									"cos":3}),
		"ReorgSplitTriangles":(240,0,192,{"add":102,
										  "mul":12,
										  "div":1,
										  "cmp":23,
										  "bit":9}),
		"writeFinalTriangles":(36,0,12,{"cmp":2,
										"div":1}),
		"CheckTriangleCollide":(32,72,16,{"add":81,
										  "mul":73,
										  "div":4,
										  "push":1,
										  "bit":13,
										  "cmp":18}),
		"Collide":(64,128,0,{"add":70,
							 "push":2,
							  "mul":81,
							  "bit":18,
							  "cmp":16}),
		"updateCurrentNode":(24,192,64,{"add":66,
										"mul":84,
										"cmp":3,
										"bit":1})}
		
invocations={}
time = {}
alias = {"NanToBoolRight":"InitialScan",
		 "NanToRight":"Scan",
		 "linearReorgSplitTriangles":"ReorgSplitTriangles",
		 "EstablishGuess":"BeginGatherSearch",
		 "UpdateGuess":"GatherSearch",
		 "NanToBoolRight2":"InitialScan2",
		 "NanToRight2":"Scan2",
		 "EstablishGuess2":"BeginGatherSearch2",
		 "UpdateGuess2":"GatherSearch2",
		 "NanToBoolRight3":"InitialScan3",
		 "NanToRight3":"Scan3",
		 "EstablishGuess3":"BeginGatherSearch3",
		 "UpdateGuess3":"GatherSearch3",
		 "NanToBoolRight4":"InitialScan4",
		 "NanToRight4":"Scan4",
		 "EstablishGuess4":"BeginGatherSearch4",
		 "UpdateGuess4":"GatherSearch4"}

for arg in sys.argv[1:]:
	invocations={}
	time = {}	
	fp = open(arg,"r");
	lines = fp.readlines();
	for line in lines:
		line = line.strip();
		where=line.find(' ')
		if (where!=-1):
			nam = line[0:where]
			num = int(line[where+1:])
			if (nam in alias):
				nam = alias[nam];
			if not nam in invocations:
				invocations[nam]=0;
			invocations[nam]+=num;
	fp.close();
	voutreads=0
	voutwrites=0
	voutarith=0
	voutmem=0
	voutscatreads=0
	voutscatwrites=0
	voutscatarith=0
	voutscatmem=0
	procreads=0
	procwrites=0
	procmem=0
	procarith=0
	for func in invocations:
		numpixels = invocations[func];
		memarith=costs[func];
		#print func + " blah "+str(memarith[2])
		reads = numpixels*(memarith[0]+memarith[1]);
		writes = numpixels*memarith[2];
		mem = reads+writes;
		#print func + " blahmem "+str(mem)
		st= func +": rd: "+str(reads)+" wr: "+str(writes);
		st += " mem: "+str(mem);
		arith = 0
		for i in memarith[3]:
			arith+=memarith[3][i];
		arith = numpixels*arith;
		st += " arith: "+str(arith);
		if (func in vout):
			voutreads+=reads
			voutwrites+=writes
			voutmem+=mem
			voutarith+=arith
		if (func in voutscatter):
			voutscatreads+=reads
			voutscatwrites+=writes
			voutscatmem+=mem
			voutscatarith+=arith
		if ((not func in vout) and (not func in voutscatter)):
			procreads+=reads
			procwrites+=writes
			procmem+=mem
			procarith+=arith
		#print st
	print arg+" "+str (procmem)+" "+str (voutmem)+" "+str (voutscatmem)+" "+str (procarith)+" "+str (voutarith)+" "+str (voutscatarith);
	print "Totals-"+arg+" "+str (procmem)+" "+str (procmem+voutmem)+" "+str (procmem+voutscatmem)+" "+str (procarith)+" "+str (procarith+voutarith)+" "+str (procarith+voutscatarith);
#	print "Mem: "+str (procmem+voutmem)
#	print "Ath: "+str (procarith+voutarith)
#	print ""
#	print "Proc"
#	print "Mem Arith: "+str (procmem)+" "+str (procarith)
#	print ""
#	print "Vout"
#	print "Mem Arith: "+str (voutmem)+" "+str (voutarith)
#	print ""
#	print "Vout Scatter"
#	print "Mem Arith: "+str (voutscatmem)+" "+str (voutscatarith)



