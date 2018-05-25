import chess.board.Board;
import chess.piece.*;

import java.util.ArrayList;
import java.util.List;

public class Driver
{
    public static void main(String[] args)
    {
        Board board = new Board();
        System.out.println(board.getSquareAt(0));
        System.out.println(board.getPieceAt(4));
        System.out.println(board.getPieceAt('e', 8));
    }
}
