#include "uci.h"

#include <iostream>
#include <sstream>

Move UCI::parseMove(const std::string& text)
{
    if (text.size() < 4)
    {
        return {64, 64, EMPTY};
    }

    int fromFile = text[0] - 'a';
    int fromRank = text[1] - '0';
    int toFile = text[2] - 'a';
    int toRank = text[3] - '0';

    if (fromFile < 0 || fromFile > 7 ||
        toFile < 0 || toFile > 7 ||
        fromRank < 1 || fromRank > 8 ||
        toRank < 1 || toRank > 8)
    {
        return {64, 64, EMPTY};
    }

    Move move;

    move.from = (8 - fromRank) * 8 + fromFile;
    move.to = (8 - toRank) * 8 + toFile;
    move.promotion = EMPTY;

    if (text.size() == 5)
    {
        bool white = board.getTurn() == WHITE;

        switch (text[4])
        {
            case 'q':
                move.promotion = white ? wQueen : bQueen;
                break;

            case 'r':
                move.promotion = white ? wRook : bRook;
                break;

            case 'b':
                move.promotion = white ? wBishop : bBishop;
                break;

            case 'n':
                move.promotion = white ? wKnight : bKnight;
                break;

            default:
                return {64, 64, EMPTY};
        }
    }

    return move;
}

std::string UCI::moveToString(const Move& move) const
{
    if (move.from >= 64 || move.to >= 64)
    {
        return "0000";
    }

    std::string text;

    text += static_cast<char>('a' + move.from % 8);
    text += static_cast<char>('8' - move.from / 8);
    text += static_cast<char>('a' + move.to % 8);
    text += static_cast<char>('8' - move.to / 8);

    switch (move.promotion)
    {
        case wQueen:
        case bQueen:
            text += 'q';
            break;

        case wRook:
        case bRook:
            text += 'r';
            break;

        case wBishop:
        case bBishop:
            text += 'b';
            break;

        case wKnight:
        case bKnight:
            text += 'n';
            break;
    }

    return text;
}

void UCI::setPosition(const std::string& command)
{
    std::istringstream input(command);
    std::string token;

    input >> token; // "position"
    input >> token; // "startpos" or "fen"

    if (token == "startpos")
    {
        board.reset();
    }
    else if (token == "fen")
    {
        /*
         * Add board.loadFEN(...) later.
         * For the first version, support startpos only.
         */
        return;
    }

    // Find the optional "moves" part.
    while (input >> token)
    {
        if (token == "moves")
        {
            break;
        }
    }

    while (input >> token)
    {
        Move move = parseMove(token);

        if (move.from >= 64 || !board.isLegal(move))
        {
            std::cerr << "Invalid UCI move: " << token << '\n';
            return;
        }

        board.move(move);
    }
}

void UCI::go(const std::string& command)
{
    std::istringstream input(command);
    std::string token;

    int depth = 5;

    input >> token; // "go"

    while (input >> token)
    {
        if (token == "depth")
        {
            input >> depth;
        }
    }

    Move bestMove = search.findBestMove(board, depth);

    std::cout << "info depth " << depth
              << " nodes " << search.getNodes()
              << '\n';

    std::cout << "bestmove "
              << moveToString(bestMove)
              << std::endl;
}

void UCI::run()
{
    std::string command;

    while (std::getline(std::cin, command))
    {
        if (command == "uci")
        {
            std::cout << "id name Rash-bot\n";
            std::cout << "id author Amrit Bhasin\n";
            std::cout << "uciok" << std::endl;
        }
        else if (command == "isready")
        {
            std::cout << "readyok" << std::endl;
        }
        else if (command == "ucinewgame")
        {
            board.reset();
        }
        else if (command.rfind("position ", 0) == 0)
        {
            setPosition(command);
        }
        else if (command.rfind("go", 0) == 0)
        {
            go(command);
        }
        else if (command == "quit")
        {
            break;
        }
    }
}