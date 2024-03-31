
enum Type {
    NONE = 0,
    PAWN,
    ROOK,
    BISHOP,
    KNIGHT,
    QUEEN,
    KING,
};

enum Color {
    BLACK = 1,
    WHITE,
};

struct Piece {
    enum Type type;
    enum Color color;
};

int board[8][8] = {
  {},
  {},
  {},
  {},
  {},
  {},
  {},
  {},
};

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
