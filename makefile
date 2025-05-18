CC = gcc
CFLAGS = -Wall -Wextra
TARGETS = treasure_manager treasure_hub score_calculator

all: $(TARGETS)

treasure_manager: treasure_manager.c treasure.h
	$(CC) $(CFLAGS) -o treasure_manager treasure_manager.c

treasure_hub: treasure_hub.c treasure.h
	$(CC) $(CFLAGS) -o treasure_hub treasure_hub.c

score_calculator: score_calculator.c treasure.h
	$(CC) $(CFLAGS) -o score_calculator score_calculator.c

clean:
	rm -f $(TARGETS) *.o

rebuild: clean all
