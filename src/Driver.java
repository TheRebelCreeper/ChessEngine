import chess.piece.*;

import java.util.ArrayList;
import java.util.List;

public class Driver
{
    public static void main(String[] args)
    {
        List<Piece> whitePieces = new ArrayList<>();
        whitePieces.add(new Rook(0));
        whitePieces.add(new Knight(1));
        whitePieces.add(new Bishop(2));
        whitePieces.add(new Queen(3));
        whitePieces.add(new King(4));
        whitePieces.add(new Bishop(5));
        whitePieces.add(new Knight(6));
        whitePieces.add(new Rook(7));

        for(int i = 0; i < 8; i++)
        {
            whitePieces.add(new Pawn(8 + i));
        }

        List<Piece> blackPieces = new ArrayList<>();
        blackPieces.add(new Rook(56));
        blackPieces.add(new Knight(57));
        blackPieces.add(new Bishop(58));
        blackPieces.add(new Queen(59));
        blackPieces.add(new King(60));
        blackPieces.add(new Bishop(61));
        blackPieces.add(new Knight(62));
        blackPieces.add(new Rook(63));

        for(int i = 0; i < 8; i++)
        {
            blackPieces.add(new Pawn(48 + i));
        }

        for (Piece p: whitePieces)
        {
            System.out.print(p + " ");
        }
        System.out.println();
        for (Piece p: blackPieces)
        {
            System.out.print(p + " ");
        }
    }
}
