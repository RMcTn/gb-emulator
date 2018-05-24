CC = gcc
CFLAGS = -g -Wall -Wextra

TARGET = gbc

SRCDIR = src
OBJDIR = obj
SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

gbc: $(OBJ)
	$(CC) $(CFLAGS)	-o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf $(OBJ) $(TARGET) $(OBJDIR)
