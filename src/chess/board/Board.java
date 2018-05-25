package chess.board;

import chess.piece.*;

import java.util.ArrayList;
import java.util.List;

public class Board
{
    private final int BOARD_SIZE = 64;
    private Square[] squares = new Square[BOARD_SIZE];

    public Board()
    {
        setupSquares();
        setupPieces();
    }

    public String getColorAtPosition(int i)
    {
        if (squares[i] instanceof LightSquare)
        {
            return "light";
        }
        else
        {
            return "dark";
        }
    }

    public Piece getPieceAt(int i)
    {
        return squares[i].getPiece();
    }

    public Piece getPieceAt(char file, int rank)
    {
        int column;
        switch(file){
            case 'a':
                column = 0;
                break;
            case 'b':
                column = 1;
                break;
            case 'c':
                column = 2;
                break;
            case 'd':
                column = 3;
                break;
            case 'e':
                column = 4;
                break;
            case 'f':
                column = 5;
                break;
            case 'g':
                column = 6;
                break;
            case 'h':
                column = 7;
                break;
                default:
                    column = 0;
        }
        return squares[(8 * (rank - 1)) + column].getPiece();
    }

    public Square getSquareAt(int i)
    {
        return squares[i];
    }

    private void setupPieces()
    {
        squares[0].setPiece(new Rook(1));
        squares[1].setPiece(new Knight(1));
        squares[2].setPiece(new Bishop(1));
        squares[3].setPiece(new Queen(1));
        squares[4].setPiece(new King(1));
        squares[5].setPiece(new Bishop(1));
        squares[6].setPiece(new Knight(1));
        squares[7].setPiece(new Rook(1));

        for(int i = 0; i < 8; i++)
        {
            squares[8 + i].setPiece(new Pawn(1));
        }

        squares[56].setPiece(new Rook(-1));
        squares[57].setPiece(new Knight(-1));
        squares[58].setPiece(new Bishop(-1));
        squares[59].setPiece(new Queen(-1));
        squares[60].setPiece(new King(-1));
        squares[61].setPiece(new Bishop(-1));
        squares[62].setPiece(new Knight(-1));
        squares[63].setPiece(new Rook(-1));

        for(int i = 0; i < 8; i++)
        {
            squares[48 + i].setPiece(new Pawn(-1));
        }
    }

    private void setupSquares()
    {
        for (int i = 0; i < BOARD_SIZE; i++)
        {
            if ((i / 8) % 2 == 0)
            {
                if (i % 2 == 0)
                {
                    squares[i] = new DarkSquare(i);
                }
                else
                {
                    squares[i] = new LightSquare(i);
                }
            }
            else
            {
                if (i % 2 == 0)
                {
                    squares[i] = new LightSquare(i);
                }
                else
                {
                    squares[i] = new DarkSquare(i);
                }
            }
        }
    }
}
