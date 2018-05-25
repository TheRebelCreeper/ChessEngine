package chess.piece;

public abstract class Piece
{
    private final double value;
    private final int color;

    public Piece(double value, int color)
    {
        this.value = value;
        this.color = color;
    }

    public double getValue()
    {
        return value;
    }

    @Override
    public String toString()
    {
        return getClass().getName() + "(value = " + value + ", color = " + color + ")";
    }
}
