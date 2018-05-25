package chess.board;

import chess.piece.Piece;

public abstract class Square
{
    private Piece piece;
    private int position;
    private final char[] files = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

    public Square(int position)
    {
        piece = null;
        if (position < 0 || position > 63)
        {
            throw new IndexOutOfBoundsException();
        }
        this.position = position;
    }

    public void setPiece(Piece p)
    {
        this.piece = p;
    }

    public Piece getPiece()
    {
        return piece;
    }

    @Override
    public String toString()
    {
        return files[position % 8] + "" + ((position / 8) + 1);
    }
}
