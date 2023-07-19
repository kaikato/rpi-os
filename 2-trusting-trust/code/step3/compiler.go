package main

import (
	"fmt"
	"os"
	"os/exec"
	"strings"
)

func notmain() {
	/*
		if os.Args[2] != "-o" {
			fmt.Fprintf(os.Stderr, strings.ToLower("expected -o as second argument, have <%s>\n"), os.Args[2])
			os.Exit(1)
		}
	*/
	file, err := os.Open(os.Args[1])
	if err != nil {
		fmt.Fprintf(os.Stderr, "file <%s> does not exist\n", os.Args[1])
		os.Exit(1)
	}
	defer file.Close()

	fileInfo, _ := file.Stat()
	fileSize := fileInfo.Size()
	if fileSize > 8*1024*1024 {
		fmt.Fprintf(os.Stderr, "input file too large\n")
		os.Exit(1)
	}

	program := make([]byte, fileSize)
	_, err = file.Read(program)
	if err != nil {
		fmt.Fprintf(os.Stderr, "invalid read of file\n")
		os.Exit(1)
	}

	// compile
	tempFile, err := os.Create("./temp-out.go")
	if err != nil {
		fmt.Fprintf(os.Stderr, strings.ToLower("Error creating temp file"))
		os.Exit(1)
	}
	defer tempFile.Close()

	err = os.WriteFile("./temp-out.go", program, 0644)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error writing to temp file\n")
		os.Exit(1)
	}

	lastChar := os.Args[1][len(os.Args[1])-1:]
	cmd := exec.Command("go", "build", "-o", os.Args[3], tempFile.Name())
	if lastChar == "c" {
		e := os.Rename("temp-out.go", "temp-out.c")
		if e != nil {
			os.Exit(1)
		}
		cmd = exec.Command("gcc", "temp-out.c", "-o", os.Args[3])
		fmt.Println(cmd)
	}
	fmt.Println(cmd)
	err = cmd.Run()
	if err != nil {
		fmt.Fprintf(os.Stderr, "system failed: %v\n", err)
		os.Exit(1)
	}
}
