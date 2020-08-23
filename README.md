# Connect 4

This is a C program where a parent process plays the game connect 4 with `n` child processes. The parent process will place their chip, and then moves onto the next child, while the previous child makes their move. The connect 4 AI is not in depth; the parent process simply picks a single column to place all of its chips into for each child. Each child will randomly place chips in columns that are the not the parent's chosen column.

## Dependencies

- `gcc 9.3.0`

### C Dependencies

- `csapp.c`
- `csapp.h`

## Compilation

`make` or `make build` to compile the C code.

## Usage

`make run GAMES=<num_of_games> DIMENSION=<dim_of_board>` or `./connect4 <num_of_games> <dim_of_board>` to run the game(s).

Note: The number of games is the same as the number of children the parent plays against.
