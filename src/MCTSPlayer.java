/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package put.ai.games.mctsplayer;

import java.util.*;

import put.ai.games.game.Board;
import put.ai.games.game.Move;
import put.ai.games.game.Player;
import put.ai.games.migration.MigrationBoard;
import put.ai.games.migration.MigrationMove;
import java.lang.Math;

public class MCTSPlayer extends Player {

    private final double MCTS_VIRTUAL_LOSS = 1;
    private final double MCTS_PARENT_VIRTUAL_LOSS = 1;
    private final double MCTS_EXPLORE_CONSTANT = 2 * Math.sqrt(2);
    private final int ATTACK_DEFEND_RATIO = 17;
    private final double THINK_TIME_RATIO = 0.85;

    private int round = 0;

    private Random random = new Random(System.currentTimeMillis());

    @Override
    public String getName() {
        return "Final Agent";
    }

    @Override
    public Move nextMove(Board b) {
        long startTime = System.currentTimeMillis();
        long endTime = startTime + (long) (getTime() * THINK_TIME_RATIO);


        round++;
        if (round == 1) {
            return getFirstMove((MigrationBoard) b);
        } else {
            MCTSEngine mcts = new MCTSEngine(b, getColor(), random);
            MigrationMove move = mcts.findBestMove(endTime);
            if (move != null) return move;
            else return b.getMovesFor(getColor()).get(0);
        }
    }

    MigrationMove getFirstMove(MigrationBoard b) {
        if (getColor() == Color.PLAYER1){
            MigrationMove bestMove = null;
            int maxRow = -1;
            int maxCol = -1;
            // select Middle point as start
            for (Move move : b.getMovesFor(getColor())) {
                MigrationMove migrationMove = (MigrationMove) move;
                if (migrationMove.getSrcX() > maxCol || (migrationMove.getSrcX() == maxCol && migrationMove.getSrcY() > maxRow)) {
                    bestMove = migrationMove;
                    maxCol = bestMove.getSrcX();
                    maxRow = bestMove.getSrcY();
                }
            }

            return bestMove;
        }
        else if (getColor() == Color.PLAYER2) {
            MigrationMove bestMove = null;
            int minRow = b.getSize();
            int minCol = b.getSize();
            // select Middle point as start
            for (Move move : b.getMovesFor(getColor())) {
                MigrationMove migrationMove = (MigrationMove) move;
                if (migrationMove.getSrcY() < minRow || (migrationMove.getSrcY() == minRow && migrationMove.getSrcX() < minCol)) {
                    bestMove = migrationMove;
                    minCol = bestMove.getSrcX();
                    minRow = bestMove.getSrcY();
                }
            }
            return bestMove;
        }
        else return null;
    }

    class MCTSNode {
        int winCount = 0;
        int allCount = 0;
        boolean isTerminal = false;

        boolean isWinning = false; // if terminal
        int move = FastMigrationBoard.MOVE_INVALID;

        MCTSNode parent = null;
        ArrayList<MCTSNode> childs = null;
    }

    class MCTSEngine {

        MCTSNode root;
        MigrationBoard rootBoard;
        Color rootColor;
        Random random;
        long cntNodes;
        MigrationMove bestMove;
        long bestMoveWinCount;

        MCTSEngine(Board board, Color color, Random rng) {
            random = rng;
            rootBoard = (MigrationBoard) board.clone();
            root = new MCTSNode();
            cntNodes = 1;
            rootColor = color;
            bestMoveWinCount = -1;
            bestMove = null;
        }

        MigrationMove findBestMove(long endTime) {
            while (System.currentTimeMillis() < endTime) {
                performIteration(endTime);
            }
            return bestMove;
        }

        void performIteration(long endTime) {
            FastMigrationBoard board = new FastMigrationBoard(rootBoard, rootColor, random);

            // Selection
            MCTSNode parent = root;
            while (parent.childs != null) {
                MCTSNode child = SelectUCT(board, parent);
                parent = child;
            }
            if (System.currentTimeMillis() >= endTime) return;

            if (board.gameEnd()) {
                parent.isTerminal = true;
                parent.isWinning = board.amIWinner();
            }

            if (parent.isTerminal) {
                Backproagate(board, parent, parent.isWinning);
            } else {
                Expand(board, parent);

                if (System.currentTimeMillis() >= endTime) return;
                MCTSNode node = SelectRandom(board, parent);

                boolean win = false;
                if (board.gameEnd()) {
                    node.isTerminal = true;
                    win = node.isWinning = board.amIWinner();
                } else {
                    if (System.currentTimeMillis() >= endTime) return;
                    win = board.simulateRandom(1, ATTACK_DEFEND_RATIO);
                }

                if (System.currentTimeMillis() >= endTime) return;
                Backproagate(board, node, win);
            }
        }

        void Backproagate(FastMigrationBoard board, MCTSNode node, boolean win) {

            MCTSNode prev = node;
            while (node.parent != null) {
                node.allCount += 1;
                node.winCount += win? 1 : 0;
                prev = node;
                node = node.parent;
            }

            root.allCount += 1;
            root.winCount += win? 1 : 0;


            if (win &&  bestMoveWinCount < prev.winCount) {
                bestMove = board.moveToMigrationMoveFromRootPerspetive(prev.move);
                bestMoveWinCount = prev.winCount;
            }
        }

        void Expand(FastMigrationBoard board, MCTSNode node) {
            List<Integer> moves = board.getMovesList();
            node.childs = new ArrayList<>();
            for (Integer move : moves) {
                cntNodes++;
                MCTSNode child = new MCTSNode();
                child.parent = node;
                child.move = move;
                node.childs.add(child);
            }
        }


        private MCTSNode SelectRandom(FastMigrationBoard board, MCTSNode node) {
            MCTSNode child = node.childs.get(random.nextInt(node.childs.size()));
            board.makeMove(child.move);
            return child;
        }

        private double UCT(MCTSNode node) {
            return (double) node.winCount / (node.allCount + MCTS_VIRTUAL_LOSS) + MCTS_EXPLORE_CONSTANT * Math.sqrt(
                    Math.log(node.parent.allCount + MCTS_PARENT_VIRTUAL_LOSS) / (node.allCount + MCTS_VIRTUAL_LOSS)
            );
        }

        private MCTSNode SelectUCT(FastMigrationBoard board, MCTSNode node) {

            double bestValue = UCT(node.childs.get(0));
            MCTSNode bestNode = node.childs.get(0);

            for (MCTSNode child : node.childs) {
                double childValue = UCT(child);

                if (childValue > bestValue) {
                    bestValue = childValue;
                    bestNode = child;
                }
            }

            board.makeMove(bestNode.move);
            return bestNode;
        }
    }
}


class FastMigrationBoard {

    public static final int MOVE_INVALID = -1;
    public static final int MOVE_PLAYER_NONE = -1;
    public static final int MOVE_PLAYER1 = 0;
    public static final int MOVE_PLAYER2 = 1;

    private static final int MS_EMPTY = 0;

    private static final int MS_FLAG_DEFENDING = 1 << 0;
    private static final int MS_FLAG_WILL_DEFEND = 1 << 1;
    private static final int MS_FLAG_BLOCKED = 1 << 2;

    private static final int MS_HASHMAP_INDEX_MASK = 3;
    private static final int MS_ATTACK = 0;
    private static final int MS_DEFEND = MS_FLAG_DEFENDING;
    private static final int MS_REACH = MS_FLAG_WILL_DEFEND;
    private static final int MS_WALL = MS_FLAG_DEFENDING | MS_FLAG_WILL_DEFEND;


    int n;
    int nsq;


    Random random;

    HashSet<Integer>[][] playerMoves;
    BitSet [] playerBitset;
    int currentPlayer;
    int rootPlayer;


    // Can be static
    private static int getOpponent(int p) {
        return 1 - p;
    }

    public MigrationMove moveToMigrationMoveFromRootPerspetive(int move) {
        int forwardMove = craftForwardMove(move, rootPlayer);
        return new MigrationMove(
                move % n, move / n,
                forwardMove % n, forwardMove / n,
                (rootPlayer == MOVE_PLAYER1? Player.Color.PLAYER1 : Player.Color.PLAYER2)
        );
    }

    // Does not require Board

    private boolean isMoveAtBorder(int move, int p) {
        // Return true if selected move is at border of player reach
        if (p == MOVE_PLAYER1) return move % n == n - 1;
        else if (p == MOVE_PLAYER2) return move / n == 0;
        else return false;
    }

    private boolean isMoveAtReverseBorder(int move, int p) {
        // Return true if selected move is at first column (P1) or last row (P2)
        if (p == MOVE_PLAYER1) return move % n == 0;
        else if (p == MOVE_PLAYER2) return move / n == n - 1;
        else return false;
    }
    public int craftForwardMove(int move, int p) {
        return (p == MOVE_PLAYER1) ? move + 1 : move - n;
    }
    public int craftBackwardMove(int move, int p) {
        return (p == MOVE_PLAYER1) ? move - 1 : move + n;
    }

    public int craftDefendedMove(int move, int p) {
        // Return move on your left if P2, below if P1
        return (p == MOVE_PLAYER1) ? move + n : move - 1;
    }

    // Requires Board

    private int checkMoveState(int move) {
        // Return bitwise state of selected Position. Assumes board state is valid.
        int p = checkMovePlayer(move);
        int state = MS_EMPTY;

        if (isMoveAtBorder(move, p) || checkMovePlayer(craftForwardMove(move, p)) != MOVE_PLAYER_NONE) {
            state |= MS_FLAG_BLOCKED;
        }
        if (checkMovePlayer(craftDefendedMove(move, p)) == getOpponent(p)) {
            state |= MS_FLAG_DEFENDING;
        }
        return state;
    }

    private int checkMovePlayer(int move) {
        // Return player corresponding to position `move`
        // -1 -> NONE, 0 - P1, 1 -> P2
        return (playerBitset[MOVE_PLAYER1].get(move)? 1 : 0) + (playerBitset[MOVE_PLAYER2].get(move)? 2 : 0) - 1;
    }

    public boolean gameEnd() {
        return playerMoves[currentPlayer][MS_DEFEND].isEmpty() && playerMoves[currentPlayer][MS_ATTACK].isEmpty();
    }

    public int getWinner() {
        return (gameEnd()? getOpponent(currentPlayer) : MOVE_PLAYER_NONE);
    }
    public boolean amIWinner() {
        return (getWinner() == rootPlayer);
    }


    // Modifies Board

    public boolean makeMove(int move) {
        // Expects valid move and current player

        int moveState = checkMoveState(move);
        // Make sure state is not blocked - move is possible

        // Make sure current player is the same

        int opponent = getOpponent(currentPlayer);


        // remove this move from bitset
        // mark +1 move in bitset
        // remove this move from playerMoves
        // add move to playerMoves if valid
        // add move down / left if not blocking
        // remove next if blocking

        int forwardMove = craftForwardMove(move, currentPlayer);
        // assedrt we can go forward (expects possible move)

        playerBitset[currentPlayer].clear(move);
        playerBitset[currentPlayer].set(forwardMove);

        int forwardMoveState = checkMoveState(forwardMove);
        if ((forwardMoveState & MS_FLAG_BLOCKED) == 0) {
            playerMoves[currentPlayer][forwardMoveState & MS_HASHMAP_INDEX_MASK].add(forwardMove);
        } else if (!isMoveAtBorder(forwardMove, currentPlayer)) {
            int moveAfterForward = craftForwardMove(forwardMove, currentPlayer);
            // BLOCK and enemy is at the next block
            // We must ensure to change the state of enemy state (if it exists):
            if (checkMovePlayer(moveAfterForward) == opponent) {
                int currentEnemyDefenseState = checkMoveState(moveAfterForward) & MS_HASHMAP_INDEX_MASK;
                int prevEnemyDefenseState = currentEnemyDefenseState & ~MS_FLAG_DEFENDING;
                // it might be blocked by something else?
                if (playerMoves[opponent][prevEnemyDefenseState].contains(moveAfterForward)) {
                    playerMoves[opponent][prevEnemyDefenseState].remove(moveAfterForward);
                    playerMoves[opponent][currentEnemyDefenseState].add(moveAfterForward);
                }
            }
        }

        playerMoves[currentPlayer][moveState & MS_HASHMAP_INDEX_MASK].remove(move);

        // This adds new move on our last defending position
        if ((moveState & MS_FLAG_DEFENDING) != 0) {
            int newMove = craftDefendedMove(move, currentPlayer);
            if (checkMovePlayer(newMove) == opponent) {
                int newMoveType = checkMoveState(newMove);
                playerMoves[opponent][newMoveType & MS_HASHMAP_INDEX_MASK].add(newMove);
            }
        }


        // This removes move if necessary for our future position
        if ((forwardMoveState & MS_FLAG_DEFENDING) != 0) {
            int deleteMove = craftDefendedMove(forwardMove, currentPlayer);
            if (checkMovePlayer(deleteMove) == opponent) {
                int deleteMoveState = checkMoveState(deleteMove);
                playerMoves[opponent][deleteMoveState & MS_HASHMAP_INDEX_MASK].remove(deleteMove);
            }
        }

        // add backward move if I exist:
        if (!isMoveAtReverseBorder(move, currentPlayer)) {
            int backwardMove = craftBackwardMove(move, currentPlayer);
            if (checkMovePlayer(backwardMove) == currentPlayer) {
                int backwardMoveState = checkMoveState(backwardMove);
                playerMoves[currentPlayer][backwardMoveState & MS_HASHMAP_INDEX_MASK].add(backwardMove);
            }
        }


        currentPlayer = getOpponent(currentPlayer);
        return true;
    }

    public boolean simulateRandom(int weightDefend, int weightAttack)  {
        while (!gameEnd()) {
            //display();

            int winningType = (playerMoves[currentPlayer][MS_ATTACK].size() == 0 ||
                    (playerMoves[currentPlayer][MS_DEFEND].size() > 0 && random.nextInt(weightDefend + weightAttack) < weightDefend)) ? MS_DEFEND : MS_ATTACK;


            int randomIdx = random.nextInt(playerMoves[currentPlayer][winningType].size());
            Iterator<Integer> iterator = playerMoves[currentPlayer][winningType].iterator();
            for (int i = 0; i < randomIdx - 1; i++) {
                iterator.next();
            }
            Integer move = iterator.next();

            boolean success = makeMove(move);

            // try { Thread.sleep(2000); } catch (Exception ignored) {            }
        }
        return amIWinner();
    }

    String getPlayerName(int p) {
        if (p == MOVE_PLAYER1) {
            return "PLAYER1";
        } else if (p == MOVE_PLAYER2) {
            return "PLAYER2";
        } else if (p == MOVE_PLAYER_NONE) {
            return "NONE";
        } else {
            return "????";
        }
    }

    public void display() {
        for (int row = 0; row < n; row++) {
            for (int col = 0; col < n; col++) {
                int move = row * n + col;
                int p = checkMovePlayer(move);
                if (p == MOVE_PLAYER_NONE) {
                } else {
                }
            }
        }


        for (int p = 0; p <= 1; p++) {
            for (int type = MS_ATTACK; type <= MS_DEFEND; type++) {
                for (Integer move : playerMoves[p][type]) {
                }
            }
        }

    }

    FastMigrationBoard(MigrationBoard board, Player.Color color, Random rand) {
        random = rand;
        n = board.getSize();
        nsq = n * n;
        playerBitset = new BitSet[]{ new BitSet(nsq), new BitSet(nsq) };
        playerMoves = new HashSet[2][2];


        playerMoves[0][0] = new HashSet<>();
        playerMoves[0][1] = new HashSet<>();
        playerMoves[1][0] = new HashSet<>();
        playerMoves[1][1] = new HashSet<>();


        currentPlayer = (color == Player.Color.PLAYER1) ? MOVE_PLAYER1 : MOVE_PLAYER2;
        rootPlayer = currentPlayer;

        for (int row = 0; row < n; row++) {
            for (int col = 0; col < n; col++) {
                if (board.getState(col, row) == Player.Color.PLAYER1) {
                    playerBitset[MOVE_PLAYER1].set(row * n + col);
                } else if (board.getState(col, row) == Player.Color.PLAYER2) {
                    playerBitset[MOVE_PLAYER2].set(row * n + col);
                }
            }
        }

        for (int move = 0; move < nsq; move++) {
            int p = checkMovePlayer(move);
            if (p == MOVE_PLAYER_NONE) continue;

            int moveState = checkMoveState(move);
            if ((moveState & MS_FLAG_BLOCKED) == 0) {
                playerMoves[p][moveState & MS_HASHMAP_INDEX_MASK].add(move);
            }
        }
    }

    public List<Integer> getMovesList() {
        List<Integer> moves = new ArrayList<>();
        moves.addAll(playerMoves[currentPlayer][MS_ATTACK]);
        moves.addAll(playerMoves[currentPlayer][MS_DEFEND]);
        return moves;
    }
}
