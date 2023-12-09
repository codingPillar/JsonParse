OUT_NAME:=main

$(OUT_NAME): src/main.cpp
	g++ -Wall -Wextra $^ -o $@ -lgtest

clean:
	rm -rf $(OUT_NAME)