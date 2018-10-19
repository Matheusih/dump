import java.util.ArrayList;

public class Node {
	public static int STEP = 10; //StepCost (The G cost)
	Node Parent = null;
	int H;
	int F;
	int x,y;
	boolean blocked = false;
	public Node(int x, int y) {
		this.x = x;
		this.y = y;
	}
	
	public ArrayList<Node> getNeighbors(Node[][] Grid) {
		/** Returns 4-adjacency neighbors of node target  **/
		ArrayList<Node> neighbors = new ArrayList<Node>();
		int x = this.x;
		int y = this.y;
		System.out.println(x + " " + y);
		if(x+1 < AStar.M) neighbors.add(Grid[x+1][y]);
		if(x-1 >= 0) neighbors.add(Grid[x-1][y]);
		if(y+1 < AStar.M) neighbors.add(Grid[x][y+1]);
		if(y-1 >= 0) neighbors.add(Grid[x][y-1]);
		
		for(Node node : neighbors)  // sets current Position to Parent its neighbors
			node.Parent = this;
		
		return neighbors;
	}
}
