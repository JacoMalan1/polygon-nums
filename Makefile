all:
	@echo " CPP  main.cpp"
	@g++ main.cpp -lpthread -lgmpxx -lgmp -o shape_nums

clean:
	@echo " RM  shape_nums"
	@rm shape_nums
