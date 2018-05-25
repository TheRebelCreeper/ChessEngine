package chess.piece;

public abstract class Piece
{
    private final double value;
    private int position;
    private final char[] files = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

    /**
     *
     * @param position Must be >= 0 and < 64
     * @param value
     */
    public Piece(int position, double value)
    {
        if (position < 0 || position > 63)
        {
            throw new IndexOutOfBoundsException();
        }
        this.position = position;
        this.value = value;
    }

    public double getValue()
    {
        return value;
    }

    @Override
    public String toString()
    {
        return files[position % 8] + "" + ((position / 8) + 1);
    }
}
