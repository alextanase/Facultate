#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// A structure to represent an adjacency list node

#define INF 1000000
struct AdjListNode
{
    int nodeout;
    struct AdjListNode* next;
    int port;
    char stare_port[2];
    int cost;
    int min;
};
 
// A structure to represent an adjacency list
struct AdjList
{
    struct AdjListNode *head;  // pointer to head node of list
};
 
// A structure to represent a graph. A graph is an array of adjacency lists.
// Size of array will be V (number of vertices in graph)
struct Graph
{
    int V;
    struct AdjList* array;
};
 
// A utility function to create a new adjacency list node
struct AdjListNode* newAdjListNode(int nodeout)
{
    struct AdjListNode* newNode =
            (struct AdjListNode*) malloc(sizeof(struct AdjListNode));
    newNode->nodeout = nodeout;
    newNode->next = NULL;
    return newNode;
}
 
// A utility function that creates a graph of V vertices
struct Graph* createGraph(int V)
{
    struct Graph* graph = (struct Graph*) malloc(sizeof(struct Graph));
    graph->V = V;
 
    // Create an array of adjacency lists.  Size of array will be V
    graph->array = (struct AdjList*) malloc(V * sizeof(struct AdjList));
 
     // Initialize each adjacency list as empty by making head as NULL
    int i;
    for (i = 0; i < V; ++i)
        graph->array[i].head = NULL;
 
    return graph;
}
 
// Adds an edge to an undirected graph
void addEdge(struct Graph* graph, int src, int nodeout, int port1,int port2,int c)
{
    // Add an edge from src to nodeout.  A new node is added to the adjacency
    // list of src.  The node is added at the begining
    struct AdjListNode* newNode = newAdjListNode(nodeout);
    newNode->next = graph->array[src].head;
    newNode->port = port1;
    newNode->cost = c;
    graph->array[src].head = newNode;
 
    // Since graph is undirected, add an edge from nodeout to src also
    newNode = newAdjListNode(src);
    newNode->next = graph->array[nodeout].head;
    newNode->port = port2;
    newNode->cost = c;
    graph->array[nodeout].head = newNode;
}
// A utility function to print the adjacenncy list representation of graph
void printGraph(struct Graph* graph)
{
    int v;
    for (v = 0; v < graph->V; ++v)
    {
        struct AdjListNode* pCrawl = graph->array[v].head;
        printf("Nodul %d: ", v);
        while (pCrawl)
        {
            printf("\neste legat cu %d de la port %d cu cost %d si stare %s", pCrawl->nodeout, pCrawl->port, pCrawl->cost, pCrawl->stare_port);
            pCrawl = pCrawl->next;
        }
        printf("\n\n");
    }
}
int cerinta1(int nr_switchuri, char (*brigdeID)[30]){
   int i;
   int root_brigde = 0;
   char aux[100];
   strcpy(aux, brigdeID[0]);
   for(i = 1; i < nr_switchuri; i++){
      if(strcmp(aux, brigdeID[i]) > 0){
         strcpy(aux, brigdeID[i]);
         root_brigde = i;
      }
   }
   //printf("%d root_brigde\n", root_brigde);
   return root_brigde;
}

void cerinta2_1(struct Graph* graph,int root_brigde, int dist[],char (*brigdeID)[30], int x[]){
   int i;
   int cost_aux = 0;
   for(i = 0; i < graph->V; i++){
      struct AdjListNode* cursor = graph->array[i].head;
      struct AdjListNode* cursor2 = cursor;
      cost_aux = 0;
      //printf("Nodul %d\n", i);
      while(cursor){
         if(i == root_brigde)
            strcpy(cursor->stare_port, "DP");
         if(cursor->min == 1 && dist[i] >= cursor->cost){
            strcpy(cursor->stare_port, "RP");
         }
         if(cursor->nodeout == root_brigde && cursor->cost == dist[i])
            strcpy(cursor->stare_port, "RP");
         cursor = cursor->next;
      }
   }
   printf("\n");
}
void make_dp(struct Graph* graph){
   int i;
   for(i = 0; i < graph->V; i++)
   {
      struct AdjListNode* cursor = graph->array[i].head;
      while(cursor){
         //printf("Compar %s cu RP\n", cursor->stare_port);
         if(strcmp(cursor->stare_port,"RP") == 0)
         {
            //printf("Nod %d cNodeout %d\n", i, cursor->nodeout);
            struct AdjListNode* cursor2 = graph->array[cursor->nodeout].head;
            while(cursor2){
               if(cursor2->nodeout == i && strcmp(cursor2->stare_port, "") == 0){
                  //printf("Pun dp catre nodul %d prin port %d\n", cursor2->nodeout, cursor2->port);
                  strcpy(cursor2->stare_port,"DP");
                  break;
               }
               cursor2 = cursor2->next;
            }
         }
         cursor = cursor->next;
      }
   }
}
void make_bp(struct Graph* graph){
   int i;
   for(i = 0; i < graph->V; i++)
   {
      struct AdjListNode* cursor = graph->array[i].head;
      while(cursor){
         if(strcmp(cursor->stare_port,"DP") == 0)
         {
            struct AdjListNode* cursor2 = graph->array[cursor->nodeout].head;
            while(cursor2){
               if(cursor2->nodeout == i && strcmp(cursor2->stare_port, "") == 0){
                  strcpy(cursor2->stare_port,"BP");
                  break;
               }
               cursor2 = cursor2->next;
            }
         }
         cursor = cursor->next;
      }
   }
}
void cerinta2_2(struct Graph* graph, int root_brigde){
   int i;
   for(i = 0; i < graph->V; i++){
      struct AdjListNode* cursor = graph->array[i].head;
      make_dp(graph);
      /*while(cursor){
         if(i == root_brigde)
            strcpy(cursor->stare_port,"DP");
         if(strcmp(cursor->stare_port,"RP") != 0)
           strcpy(cursor->stare_port,"DP");
         cursor = cursor->next;
      }*/
   }
}
int calculeaza_cost(int cost){
   if(cost == 100)
      cost = 19;
   if(cost == 10)
      cost = 100;
   if(cost == 1000)
      cost = 4;
   if(cost == 10000)
      cost = 1;
   return cost;
}
// Structure to represent a min heap node
struct MinHeapNode
{
    int  v;
    int dist;
};
 
// Structure to represent a min heap
struct MinHeap
{
    int size;      // Number of heap nodes present currently
    int capacity;  // Capacity of min heap
    int *pos;     // This is needed for decreaseKey()
    struct MinHeapNode **array;
};
 
// A utility function to create a new Min Heap Node
struct MinHeapNode* newMinHeapNode(int v, int dist)
{
    struct MinHeapNode* minHeapNode =
           (struct MinHeapNode*) malloc(sizeof(struct MinHeapNode));
    minHeapNode->v = v;
    minHeapNode->dist = dist;
    return minHeapNode;
}
 
// A utility function to create a Min Heap
struct MinHeap* createMinHeap(int capacity)
{
    struct MinHeap* minHeap =
         (struct MinHeap*) malloc(sizeof(struct MinHeap));
    minHeap->pos = (int *)malloc(capacity * sizeof(int));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array =
         (struct MinHeapNode**) malloc(capacity * sizeof(struct MinHeapNode*));
    return minHeap;
}
 
// A utility function to swap two nodes of min heap. Needed for min heapify
void swapMinHeapNode(struct MinHeapNode** a, struct MinHeapNode** b)
{
    struct MinHeapNode* t = *a;
    *a = *b;
    *b = t;
}
 
// A standard function to heapify at given idx
// This function also updates position of nodes when they are swapped.
// Position is needed for decreaseKey()
void minHeapify(struct MinHeap* minHeap, int idx)
{
    int smallest, left, right;
    smallest = idx;
    left = 2 * idx + 1;
    right = 2 * idx + 2;
 
    if (left < minHeap->size &&
        minHeap->array[left]->dist < minHeap->array[smallest]->dist )
      smallest = left;
 
    if (right < minHeap->size &&
        minHeap->array[right]->dist < minHeap->array[smallest]->dist )
      smallest = right;
 
    if (smallest != idx)
    {
        // The nodes to be swapped in min heap
        MinHeapNode *smallestNode = minHeap->array[smallest];
        MinHeapNode *idxNode = minHeap->array[idx];
 
        // Swap positions
        minHeap->pos[smallestNode->v] = idx;
        minHeap->pos[idxNode->v] = smallest;
 
        // Swap nodes
        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);
 
        minHeapify(minHeap, smallest);
    }
}
 
// A utility function to check if the given minHeap is empty or not
int isEmpty(struct MinHeap* minHeap)
{
    return minHeap->size == 0;
}
 
// Standard function to extract minimum node from heap
struct MinHeapNode* extractMin(struct MinHeap* minHeap)
{
    if (isEmpty(minHeap))
        return NULL;
 
    // Store the root node
    struct MinHeapNode* root = minHeap->array[0];
 
    // Replace root node with last node
    struct MinHeapNode* lastNode = minHeap->array[minHeap->size - 1];
    minHeap->array[0] = lastNode;
 
    // Update position of last node
    minHeap->pos[root->v] = minHeap->size-1;
    minHeap->pos[lastNode->v] = 0;
 
    // Reduce heap size and heapify root
    --minHeap->size;
    minHeapify(minHeap, 0);
 
    return root;
}
 
// Function to decreasy dist value of a given vertex v. This function
// uses pos[] of min heap to get the current index of node in min heap
void decreaseKey(struct MinHeap* minHeap, int v, int dist)
{
    // Get the index of v in  heap array
    int i = minHeap->pos[v];
    // Get the node and update its dist value
    minHeap->array[i]->dist = dist;
 
    // Travel up while the complete tree is not hepified.
    // This is a O(Logn) loop
    while (i && minHeap->array[i]->dist < minHeap->array[(i - 1) / 2]->dist)
    {
        // Swap this node with its parent
        minHeap->pos[minHeap->array[i]->v] = (i-1)/2;
        minHeap->pos[minHeap->array[(i-1)/2]->v] = i;
        swapMinHeapNode(&minHeap->array[i],  &minHeap->array[(i - 1) / 2]);
 
        // move to parent index
        i = (i - 1) / 2;
    }
}
 
// A utility function to check if a given vertex
// 'v' is in min heap or not
bool isInMinHeap(struct MinHeap *minHeap, int v)
{
   if (minHeap->pos[v] < minHeap->size)
     return true;
   return false;
}
 
// A utility function used to print the solution
void printArr(int dist[], int n)
{
    printf("Vertex   Distance from Source\n");
    for (int i = 0; i < n; ++i)
        printf("%d \t\t %d\n", i, dist[i]);
}
 
// The main function that calulates distances of shortest paths from src to all
// vertices. It is a O(ELogV) function
void dijkstra(struct Graph* graph, int src, char (*brigdeID)[30])
{
    int V = graph->V;// Get the number of vertices in graph
    int dist[V];      // dist values used to pick minimum weight edge in cut
   char egal[30];
   int x[10];
   int iterator = 0;
   for(int i = 0; i < graph->V; i++)
      x[i] = 0;
    // minHeap represents set E
    struct MinHeap* minHeap = createMinHeap(V);
 
    // Initialize min heap with all vertices. dist value of all vertices 
    for (int v = 0; v < V; ++v)
    {
        dist[v] = INF;
        minHeap->array[v] = newMinHeapNode(v, dist[v]);
        minHeap->pos[v] = v;
    }
 
    // Make dist value of src vertex as 0 so that it is extracted first
    minHeap->array[src] = newMinHeapNode(src, dist[src]);
    minHeap->pos[src]   = src;
    dist[src] = 0;
    decreaseKey(minHeap, src, dist[src]);
 
    // Initially size of min heap is equal to V
    minHeap->size = V;
 
    // In the followin loop, min heap contains all nodes
    // whose shortest distance is not yet finalized.
    while (!isEmpty(minHeap))
    {
        // Extract the vertex with minimum distance value
        struct MinHeapNode* minHeapNode = extractMin(minHeap);
        int u = minHeapNode->v; // Store the extracted vertex number
         strcpy(egal, brigdeID[u]);
         //printf("nod %d :copiez %s in egal %s\n",u, brigdeID[u], egal);
        // Traverse through all adjSacent vertices of u (the extracted
        // vertex) and update their distance values
        struct AdjListNode* pCrawl = graph->array[u].head;
        struct AdjListNode* cursor = graph->array[u].head;
        //printf("ar trebui sa fie nod %d si port %d\n",pCrawl->nodeout, pCrawl->port);
        while (pCrawl != NULL)
        {
            int v = pCrawl->nodeout;
            // If shortest distance to v is not finalized yet, and distance to v
            // through u is less than its previously calculated distance
            /*if(isInMinHeap(minHeap, v) && dist[u] != INF && pCrawl->cost + dist[u] == dist[v])
            {
               printf("Aici ");
               printf("nodul %d catre %d prin port %d\n",u ,pCrawl->nodeout, pCrawl->port);
               struct AdjListNode* cursor2 = graph->array[pCrawl->nodeout].head;
               while(cursor2){
                  if(cursor2->nodeout == u){
                     printf("compar %s cu %s\n", egal, brigdeID[pCrawl->nodeout]);
                     if(strcmp(egal,brigdeID[pCrawl->nodeout]) > 0){
                        printf("safasf %d\n", pCrawl->port);
                        pCrawl->min = 1;
                        break;
                     }
                  }
                  cursor2 = cursor2->next;
               }
            }
            else*/
            if (isInMinHeap(minHeap, v) && dist[u] != INF && 
                                          pCrawl->cost + dist[u] <= dist[v])
            {
                dist[v] = dist[u] + pCrawl->cost;
               /* if(v == src)
                  strcpy(pCrawl->stare_port,"DP");
               else
                  strcpy(pCrawl->stare_port,"RP");*/
                //printf("Distanta de la root_brigde la nodul %d este %d si sunt in portul %d si asta e ceva %d\n", pCrawl->nodeout, dist[v], pCrawl->port, u);
                // update distance value in min heap also
                decreaseKey(minHeap, v, dist[v]);
               //printf("nod %d catre %d prin port%d\n",u, pCrawl->nodeout, pCrawl->port);
               cursor = graph->array[pCrawl->nodeout].head;
              //printf("Analizez lista lui %d\n", pCrawl->nodeout);
               //printf("Sunt in nodul %d si ma duc in %d prin port %d\n",pCrawl->nodeout, cursor->nodeout, cursor->port);
               
                  while(cursor != NULL){
                     if(cursor->nodeout == u){
                        cursor->min = 1;
                        x[pCrawl->nodeout] ++; 
                        //printf("Aleg drum din  nodul %d catre %d prin port %d\n",pCrawl->nodeout, u, cursor->port);
                        break;
                     }
                     //if(strcmp(pCrawl->stare_port,"DP") == 0 && strcmp(cursor->stare_port,"RP") != 0)
                     //   strcpy(cursor->stare_port,"BP");
                  cursor = cursor->next;
               }
            } 
            pCrawl = pCrawl->next;
        }
    }
    // print the calculated shortest distances
   /* for(int i = 0; i < graph->V; i++)
      printf("x:%d ", x[i]);
   printf("\n");
    printArr(dist, V);*/
    cerinta2_1(graph, src, dist,brigdeID,x);
}
void afisare_c2_1(struct Graph* graph, FILE* output){
   int i;
   fprintf(output, "RP");
   for(i = 0 ; i < graph->V; i++){
      struct AdjListNode* cursor = graph->array[i].head;
      while(cursor){
         if(strcmp(cursor->stare_port,"RP") == 0)
            fprintf(output, " %d(%d)", i, cursor->port);
         cursor = cursor->next;
      }
   }
   fprintf(output, "\n");
}
void afisare_c2_2(struct Graph* graph, FILE* output){
   int i;
   fprintf(output, "BP");
   for(i = 0 ; i < graph->V; i++){
      struct AdjListNode* cursor = graph->array[i].head;
      while(cursor){
         if(strcmp(cursor->stare_port,"BP") == 0)
            fprintf(output, " %d(%d)", i, cursor->port);
         cursor = cursor->next;
      }
   }
   fprintf(output, "\n");
}
void afisare_c2_3(Graph* graph, FILE* output){
   int i, j;
   int top[graph->V][graph->V];
   for(i = 0; i < graph->V; i++)
    for(j = 0; j < graph->V; j++)
      top[i][j] = 0;
   for(i = 0; i < graph->V; i++){
      struct AdjListNode* cursor = graph->array[i].head;
      while(cursor){
         if(strcmp(cursor->stare_port,"RP") == 0)
            top[i][cursor->nodeout] = 1;
         cursor = cursor->next;
      }
   }
   for(i = 0; i < graph->V; i++){
    for(j = 0; j < graph->V; j++)
      if(j == 0)  
         fprintf(output, "%d", top[i][j]);
      else
         fprintf(output, " %d", top[i][j]);
   fprintf(output, "\n");
   }
}
/*void cerinta4(Graph* graph, int nod1, int nod2){
   int i;
   for(i = 0; i < graph->V; i++){
      struct AdjListNode* cursor = graph->array[i].head;
      while(cursor)
         if(cursor->nnodeout == nod1 || cursor->nodeout == nod2)
            cursor = cursor->next
   }
}*/
int main(int argc, char *argv[])
{
      FILE *tasks, *initialise, *topology, *output;
      int nr_switchuri, i, auxiliar, root_brigde;
      int nodin, nodout, portin, portout, cost;
      char nin,nout,pin,pout;
      char plm[100];
      char a;
      initialise = fopen(argv[1], "r");
      topology = fopen(argv[2], "r");
      tasks = fopen(argv[3], "r");
      output = fopen(argv[4], "a");
      char comenzi[100]; //buffer comenzi
      fscanf(initialise, "%d", &nr_switchuri);
      char muchie[100]; //buffer muchie
      char brigdeID[nr_switchuri][30]; // buffer switch id
      struct Graph* graph = createGraph(nr_switchuri); //cream graful
      while( fscanf(topology, "%d", &auxiliar) != EOF ){
         nodin = auxiliar;
         fscanf(topology, "%d", &nodout);
         fscanf(topology, "%d", &portin);
         fscanf(topology, "%d", &portout);
         fscanf(topology, "%d", &cost);
         cost = calculeaza_cost(cost);
         addEdge(graph, nodin, nodout, portin, portout, cost);
      }
      fgets(plm, 100, (FILE*)initialise); // citessc aiurea
      for(i = 0; i < nr_switchuri; i++){
         fgetc((FILE*)initialise);
         fgetc((FILE*)initialise);
         fgets(brigdeID[i], 30, (FILE*)initialise);
      }
      root_brigde = cerinta1(nr_switchuri, brigdeID);
      while( fgets(comenzi, 100, (FILE*)tasks) != NULL){
        if(strcmp(comenzi,"c1\n") == 0){
            fprintf(output, "%d\n", root_brigde);
         }
         if(strncmp(comenzi, "c2-1", 4) == 0){
             dijkstra(graph, root_brigde, brigdeID);
             afisare_c2_1(graph,output);
         }
         if(strncmp(comenzi, "c2-2", 4) == 0){
            cerinta2_2(graph, root_brigde);
            make_bp(graph);
            afisare_c2_2(graph, output);
         }
         if(strncmp(comenzi, "c2-3", 4) == 0)
            afisare_c2_3(graph, output);
         if(strncmp(comenzi,"c3", 2) == 0)
            //printf("Cerinta 3\n");
         if(strncmp(comenzi,"c4", 2) == 0)
            printf("Cerinta 4\n");
      }
      //cerinta2(graph, root_brigde, output);
      //cerinta2_1(graph, root_brigde, output);
      printGraph(graph);
      //afisare_c2_2(graph, output);
      //afisare_c2_3(graph, output);
      fclose(tasks);
      fclose(initialise);
      fclose(topology);
      fclose(output);
   return 0;
}