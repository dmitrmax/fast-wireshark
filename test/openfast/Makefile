

Main.class: Main.java
	javac -cp openfast.jar $^

.PHONY: run
run: Main.class
	java -cp openfast.jar:. Main
	make -C ../ server
	timeout 3s ../server 1337 &
	sleep 1s
	timeout 1s nc -u localhost 1337 < data1.fast \
		|| test $$? -eq 124
	timeout 1s nc -u localhost 1337 < data2.fast \
		|| test $$? -eq 124

.PHONY: clean
clean:
	rm -f Main.class

