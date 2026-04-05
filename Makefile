all:
	gcc main.c core/strategy.c app/engine.c infra/data_writer.c system/scheduler.c -o bot -lpthread