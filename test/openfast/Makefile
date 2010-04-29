

Main.class: Main.java
	javac -cp openfast.jar $^

.PHONY: run
run: Main.class
	java -cp openfast.jar:. Main
	timeout 1s netcat -u localhost 1337 < data.fast \
		|| test $$? -eq 124

.PHONY: clean
clean:
	rm -f Main.class

