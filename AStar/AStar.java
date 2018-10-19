import java.util.ArrayList;

public class AStar {
	//World Grid Size MxN, standard 8x8
	public static int M = 8;
	public static int N = 8;
	
	public static Node goal = new Node(5,5);
	
	public static int Manhattan(int x1, int y1, int x2, int y2) {
		return Math.abs(x2 - x1) + Math.abs(y2 - y1);
	}
	public static Node getSmallestF(ArrayList<Node> openlist) {
		int min = -1;
		Node target = null;
		for(Node node : openlist) {
			if(node.F < min) {
				min = node.F;
				target = node;
			}
		}
		return target;
	}
	
	public static void main(String[] args) {
		
		Node[][] Grid = new Node[8][8];
		
		for (int i = 0; i < M; i++)
            for (int j = 0; j < N; j++) {
            	Grid[i][j] = new Node(i,j);
            	Grid[i][j].F = Node.STEP + Manhattan(i,j,goal.x,goal.y);
            }
		
        Node Position = Grid[0][0];  // My current Position	
		
        ArrayList<Node> OpenList = new ArrayList<Node>();  // Unchecked Nodes
        ArrayList<Node> ClosedList = new ArrayList<Node>(); //  Checked Nodes
        
        boolean goalReached = false;
        
        do {  // Main Loop
        	
        	// neighbors returnin null, why ??
        	ArrayList<Node> neighbors = Position.getNeighbors(Grid);
        	 ClosedList.add(Position);
        	 
        	for(Node neighbor : neighbors) {
        		if (ClosedList.contains(neighbor) || OpenList.contains(neighbor))
        			continue;
        		else
        			OpenList.add(neighbor);
        	}
        	
        	if(OpenList.contains(goal)) {
        		goalReached = true;
        	}
        	
        	Position = getSmallestF(OpenList); // Reparenting not necessary ??
        	
        	
        }while(goalReached == false);
        
        Node path = goal;
        while(path != null) {
        	System.out.println(path.x + " " + path.y);
        }
	}
	
}
