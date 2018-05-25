import chess.board.Board;

public class Driver
{
    public static void main(String[] args)
    {
        Board board = new Board();
        System.out.println(board.getSquareAt(0));
        System.out.println(board.getPieceAt(4));
        System.out.println(board.getPieceAt('h', 8));
    }
}
