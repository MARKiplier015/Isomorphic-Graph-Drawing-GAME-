#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <numeric>
#include <random>

using namespace std;
using namespace sf;

struct Dot
{
    RectangleShape shape;
    bool isDragging = false;
    bool isConnected = false;
    vector<VertexArray> curves;
    Text label; // Number label for the dot
};

struct Edge
{
    VertexArray line;
    Text label; // Number label for the edge
};

Vector2f calculateMidpoint(const Vector2f& startPos, const Vector2f& endPos)
{
    return Vector2f((startPos.x + endPos.x) / 2, (startPos.y + endPos.y) / 2 + 50);
}


void sortVerticesByDegree(const vector<int>& degrees, vector<int>& sortedVertices)
{
    vector<pair<int, int>> vertexDegrees;
    for (int i = 0; i < degrees.size(); ++i)
    {
        vertexDegrees.emplace_back(i, degrees[i]);
    }
    sort(vertexDegrees.begin(), vertexDegrees.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
        });

    sortedVertices.clear();
    for (const auto& vertex : vertexDegrees)
    {
        sortedVertices.push_back(vertex.first);
    }
}

const int DotSize = 20;
const int DotSpacing = 35;
const int MaxX = 800;
const int MaxY = 600;

class Line {
public:
    Vector2f p1, p2;
    bool isSelected;

    Line(Vector2f point1, Vector2f point2, bool selected)
        : p1(point1), p2(point2), isSelected(selected) {}

    void draw(RenderWindow& window) const {
        Color color = isSelected ? Color::Red : Color::Blue;
        Vertex line[] = {
           Vertex(p1, color),
            Vertex(p2, color)
        };
        window.draw(line, 2, sf::Lines);
    }
};

void drawGraph(RenderWindow& window, const vector<Vector2f>& dots, const vector<Line>& lines) {
    for (const auto& dot : dots) {
        CircleShape circle(DotSize / 2);
        circle.setFillColor(Color::Red);
        circle.setOutlineColor(Color::White);
        circle.setOutlineThickness(1.0f);
        circle.setPosition(dot.x - DotSize / 2, dot.y - DotSize / 2);
        window.draw(circle);
    }

    for (const auto& line : lines) {
        line.draw(window);
    }

    int i = 1;
    Font font;
    font.loadFromFile("C:/Users/Acer/Downloads/cooper.ttf"); // Change this to the path of a font file.

    for (const auto& dot : dots) {
        Text text(to_string(i++), font, 14);
        text.setFillColor(Color::Black);
        text.setPosition(dot.x - 6, dot.y - 10);
        window.draw(text);
    }
}

vector<vector<int>> generateRandomGraph(int numVertices, int numEdges)
{
    random_device rd;
    mt19937 gen(rd());

    vector<vector<int>> adjacencyMatrix(numVertices, vector<int>(numVertices, 0));

    while (numEdges > 0)
    {
        int v1 = uniform_int_distribution<>(0, numVertices - 1)(gen);
        int v2 = uniform_int_distribution<>(0, numVertices - 1)(gen);

        // Ensure we don't create self-loops or duplicate edges
        if (v1 != v2 && adjacencyMatrix[v1][v2] == 0)
        {
            adjacencyMatrix[v1][v2] = 1;
            adjacencyMatrix[v2][v1] = 1;
            numEdges--;
        }
    }

    return adjacencyMatrix;
}

// Function to shuffle vertices randomly while preserving the number of edges
void shuffleVerticesWithSameEdges(vector<vector<int>>& adjacencyMatrix)
{
    int numVertices = adjacencyMatrix.size();

    // Create a random permutation of indices
    vector<int> permutation(numVertices);
    for (int i = 0; i < numVertices; ++i)
    {
        permutation[i] = i;
    }
    shuffle(permutation.begin(), permutation.end(), std::mt19937(std::random_device()()));

    // Create a copy of the original adjacency matrix to store the shuffled version
    vector<vector<int>> shuffledAdjacencyMatrix(numVertices, vector<int>(numVertices, 0));

    // Transfer edges based on the permutation
    for (int i = 0; i < numVertices; ++i)
    {
        for (int j = i + 1; j < numVertices; ++j)
        {
            if (adjacencyMatrix[permutation[i]][permutation[j]] == 1)
            {
                shuffledAdjacencyMatrix[i][j] = 1;
                shuffledAdjacencyMatrix[j][i] = 1;
            }
        }
    }

    // Replace the original adjacency matrix with the shuffled one
    adjacencyMatrix = std::move(shuffledAdjacencyMatrix);
}
vector<int> calculateDegrees(const vector<vector<int>>& adjacencyMatrix) {
    int numVertices = adjacencyMatrix.size();
    vector<int> degrees(numVertices, 0);

    for (int i = 0; i < numVertices; ++i) {
        for (int j = 0; j < numVertices; ++j) {
            degrees[i] += adjacencyMatrix[i][j];
        }
    }

    return degrees;
}
bool isIsomorphic(const vector<int>& degrees1, const vector<int>& degrees2) {
    if (degrees1.size() != degrees2.size()) {
        return false;
    }

    vector<int> sortedDegrees1 = degrees1;
    vector<int> sortedDegrees2 = degrees2;
    sort(sortedDegrees1.begin(), sortedDegrees1.end());
    sort(sortedDegrees2.begin(), sortedDegrees2.end());

    return sortedDegrees1 == sortedDegrees2;
}

int main()
{
    int numVertices, numEdges;
    vector<Edge> randomEdges1, randomEdges2;
    RenderWindow window(VideoMode(800, 600), "Graph Drawing");

    random_device rd; // Obtain a random seed from the operating system
    mt19937 gen(rd()); // Create the random number generator using the random seed
    // Loading screen
    Font font;
    if (!font.loadFromFile("C:/Users/Acer/Downloads/cooper.ttf"))
    {
        cout << "Failed to load font" << endl;
        return 1;
    }

    Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("C:/Users/Acer/Downloads/background.gif"))
    {
        cout << "Failed to load background image" << endl;
        return 1;
    }

    Sprite background(backgroundTexture);
    background.setScale(1.0f, 1.0f);

    Text loadingText;
    loadingText.setFont(font);
    loadingText.setString("Graph Drawing");
    loadingText.setCharacterSize(75);
    loadingText.setFillColor(Color::White);
    loadingText.setPosition(window.getSize().x / 2 - loadingText.getGlobalBounds().width / 2, window.getSize().y / 1.70 - loadingText.getGlobalBounds().height / 1);

    RectangleShape startButton(Vector2f(250, 90));
    startButton.setFillColor(Color::Green);
    startButton.setPosition(window.getSize().x / 2 - startButton.getGlobalBounds().width / 2, window.getSize().y / 2 + 120);
    startButton.setOutlineThickness(5.0f); // Set the thickness of the outline
    startButton.setOutlineColor(Color::White);

    Text startButtonText;
    startButtonText.setFont(font);
    startButtonText.setString("S T A R T");
    startButtonText.setCharacterSize(40);
    startButtonText.setFillColor(Color::White);
    startButtonText.setPosition(startButton.getPosition().x + startButton.getGlobalBounds().width / 2 - startButtonText.getGlobalBounds().width / 2, startButton.getPosition().y + startButton.getGlobalBounds().height / 3 - startButtonText.getGlobalBounds().height / 2);

    bool loading = true;
    bool gameStarted = false;

    Clock clock;
    float loadingAnimationTimer = 0.0f;
    const float loadingAnimationDuration = 0.5f;
    int numDots = 3;
    vector<CircleShape> loadingDots(numDots);
    float dotRadius = 10.0f;
    float dotSpacing = 30.0f;
    float dotY = startButton.getPosition().y + startButton.getGlobalBounds().height / 2 - dotRadius;

    for (int i = 0; i < numDots; ++i)
    {
        loadingDots[i].setRadius(dotRadius);
        loadingDots[i].setFillColor(Color::White);
        loadingDots[i].setPosition(window.getSize().x / 2 - (dotSpacing * (numDots - 1)) / 2 + dotSpacing * i, dotY);
    }
    while (loading)
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
            {
                window.close();
                return 0;
            }
            else if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
            {
                Vector2f mousePos = Vector2f(Mouse::getPosition(window));
                if (startButton.getGlobalBounds().contains(mousePos))
                {
                    loading = false;
                    gameStarted = true;
                }
            }
        }

        float deltaTime = clock.restart().asSeconds();
        loadingAnimationTimer += deltaTime;

        if (loadingAnimationTimer >= loadingAnimationDuration)
        {
            loadingAnimationTimer = 0.0f;
            rotate(loadingDots.begin(), loadingDots.begin() + 1, loadingDots.end());
        }

        window.clear();

        window.draw(background);

        for (const auto& dot : loadingDots)
            window.draw(dot);

        window.draw(loadingText);
        window.draw(startButton);
        window.draw(startButtonText);

        window.display();
    }

    if (gameStarted)
    {
        cout << "Enter the number of vertices(4-10): ";
        while (!(cin >> numVertices) || numVertices < 4 || numVertices > 10 || cin.peek() != '\n')
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid Input! Please re-enter: ";
        }

        // Calculate the minimum and maximum number of edges based on the number of vertices
        int minEdges = 4;
        int maxEdges = min(numVertices * (numVertices - 1) / 2, 10); // Limit the maxEdges to 10 for 10 vertices

        cout << "Enter the number of edges(" << minEdges << "-" << maxEdges << "): ";
        while (!(cin >> numEdges) || numEdges < minEdges || numEdges > maxEdges || cin.peek() != '\n')
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid Input! Please re-enter: ";
        }

        vector<Dot> dots(numVertices);
        vector<Edge> edges;
        vector<unordered_set<int>> connectedDots(numVertices);
        vector<int> degrees(numVertices, 0); // Track the degree of each vertex

        bool drawingMode = true;
        Vector2f startPos;
        Vector2f endPos;
        bool isDrawing = false;
        bool loopMode = false; // Flag for loop creation mode
        int numDrawnEdges = 0;

        // Set initial positions of dots manually or based on a pattern
        const float initialX = 100.0f;
        const float initialY = 100.0f;
        const float spacing = 35.0f;
        for (int i = 0; i < numVertices; ++i)
        {
            dots[i].shape.setSize(Vector2f(20, 20)); // Set the size of the RectangleShape
            dots[i].shape.setFillColor(Color::Yellow);
            dots[i].shape.setOutlineThickness(5); // Set the thickness of the edges
            dots[i].shape.setPosition(initialX + spacing * i, initialY);
            // Create the number label
            dots[i].label.setFont(font);
            dots[i].label.setString(to_string(i + 1)); // Set the label text to the vertex number
            dots[i].label.setCharacterSize(14);
            dots[i].label.setFillColor(Color::Black);
            // Set the position of the label relative to the dot's position            dots[i].label.setPosition(dots[i].shape.getPosition() + Vector2f(5, 2));
        }
        while (window.isOpen())
        {
            Event event;
            while (window.pollEvent(event))
            {
                if (event.type == Event::Closed)
                    window.close();
                else if (event.type == Event::KeyPressed && event.key.code == Keyboard::Space)
                {
                    drawingMode = !drawingMode;
                    loopMode = false; // Turn off loop mode when switching drawing modes
                }
                else if (event.type == Event::KeyPressed && event.key.code == Keyboard::L)
                {
                    loopMode = !loopMode; // Toggle loop creation mode when 'L' key is pressed
                    drawingMode = false; // Turn off drawing mode when entering loop mode
                }
                else if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
                {
                    if (drawingMode && !loopMode)
                    {
                        startPos = Vector2f(Mouse::getPosition(window));
                        endPos = startPos;
                        isDrawing = true;
                    }
                    else if (loopMode) // Loop creation mode
                    {
                        Vector2f mousePos = Vector2f(Mouse::getPosition(window));
                        if (numDrawnEdges < numEdges)
                        {
                            // Check if any dot is clicked
                            for (auto& dot : dots)
                            {
                                if (dot.shape.getGlobalBounds().contains(mousePos))
                                {
                                    // Create a curved line that serves as a loop for the clicked vertex
                                    VertexArray loop(LinesStrip, 3);
                                    loop[0].position = dot.shape.getPosition();
                                    loop[0].color = Color::Cyan;
                                    loop[1].position = calculateMidpoint(dot.shape.getPosition(), mousePos);
                                    loop[1].color = Color::Cyan;
                                    loop[2].position = mousePos;
                                    loop[2].color = Color::Cyan;                                dot.curves.push_back(loop);
                                    dot.isConnected = true; // Mark the dot as connected to form a loop
                                    // Create the label for the edge (loop)
                                    Text label;
                                    label.setFont(font);                                 label.setString(to_string(numDrawnEdges + 1)); // Set the label text to the edge number
                                    label.setCharacterSize(12);                                    label.setFillColor(Color::Red);
                                    Vector2f labelPosition = calculateMidpoint(dot.shape.getPosition(), mousePos);
                                    labelPosition.x -= label.getGlobalBounds().width / 2;
                                    labelPosition.y -= label.getGlobalBounds().height / 2;                                  label.setPosition(labelPosition);
                                    edges.push_back({ loop, label });
                                    numDrawnEdges++;                                    degrees[dot.label.getString()[0] - '1'] += 2; // Increment the degree of the vertex
                                    cout << "Number of edges drawn: " << numDrawnEdges << endl;
                                    break; // Only one loop allowed at a time, so exit the loop after creating the loop.
                                }
                            }
                        }
                    }
                    else
                    {
                        Vector2f mousePos = Vector2f(Mouse::getPosition(window));

                        // Check if any dot is clicked
                        for (auto& dot : dots)
                        {
                            if (dot.shape.getGlobalBounds().contains(mousePos))
                            {
                                dot.isDragging = true;
                                dot.shape.setFillColor(Color::Red); // Highlight the dot being dragged
                            }
                        }
                    }
                }
                else if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Left)
                {
                    if (drawingMode && isDrawing)
                    {
                        endPos = Vector2f(Mouse::getPosition(window));
                        int startDot = -1;
                        int endDot = -1;
                        for (int i = 0; i < numVertices; ++i)
                        {
                            if (dots[i].shape.getGlobalBounds().contains(startPos))
                            {
                                startDot = i;
                            }
                            else if (dots[i].shape.getGlobalBounds().contains(endPos))
                            {
                                endDot = i;
                            }
                        }
                        if (startDot != -1 && endDot != -1 && startDot != endDot)
                        {
                            // Check if the dots are already connected or if the same dots are connected multiple times
                            if (connectedDots[startDot].count(endDot) == 0 && connectedDots[endDot].count(startDot) == 0)
                            {
                                VertexArray line(Lines, 2);
                                line[0].position = startPos;
                                line[1].position = endPos;
                                line[0].color = Color::Cyan;
                                line[1].color = Color::Cyan;
                                edges.push_back({ line });
                                // Create the label for the edge
                                Text label;
                                label.setFont(font);                                label.setString(to_string(numDrawnEdges + 1)); // Set the label text to the edge number
                                label.setCharacterSize(12);                                label.setFillColor(Color::Red);
                                // Set the position of the label closer to the edge
                                Vector2f labelPosition = calculateMidpoint(startPos, endPos);
                                labelPosition.x -= label.getGlobalBounds().width / 2;
                                labelPosition.y -= label.getGlobalBounds().height / 2;                               label.setPosition(labelPosition);
                                edges.back().label = label;                              connectedDots[startDot].insert(endDot);                               connectedDots[endDot].insert(startDot);
                                numDrawnEdges++;
                                // Increment the degree of each vertex involved in the line
                                degrees[startDot]++;
                                degrees[endDot]++;
                                cout << "Number of edges drawn: " << numDrawnEdges << endl;
                            }
                            else if (startDot != endDot && connectedDots[startDot].count(endDot) > 0 && connectedDots[endDot].count(startDot) > 0)
                            {
                                // Create a curved line between already connected vertices
                                VertexArray curve(LinesStrip, 3);
                                curve[0].position = startPos;
                                curve[0].color = Color::Cyan;
                                curve[1].position = calculateMidpoint(startPos, endPos);
                                curve[1].color = Color::Cyan;
                                curve[2].position = endPos;
                                curve[2].color = Color::Cyan;                                dots[startDot].curves.push_back(curve);                                dots[endDot].curves.push_back(curve);
                                // Create the label for the edge
                                Text label;
                                label.setFont(font);                                label.setString(to_string(numDrawnEdges + 1)); // Set the label text to the edge number
                                label.setCharacterSize(12);                               label.setFillColor(Color::Red);
                                // Set the position of the label closer to the edge
                                Vector2f labelPosition = calculateMidpoint(startPos, endPos);
                                labelPosition.x -= label.getGlobalBounds().width / 2;
                                labelPosition.y -= label.getGlobalBounds().height / 2;                              label.setPosition(labelPosition);
                                edges.push_back({ curve, label });
                                // Update the degree count for the same dots                                connectedDots[startDot].erase(endDot);                                connectedDots[endDot].erase(startDot);                                connectedDots[startDot].insert(startDot);                                connectedDots[endDot].insert(endDot);
                                numDrawnEdges++;
                                // Increment the degree of each vertex involved in the line
                                degrees[startDot]++;
                                degrees[endDot]++;
                                cout << "Number of edges drawn: " << numDrawnEdges << endl;
                            }
                        }
                        isDrawing = false;
                        if (numDrawnEdges == numEdges)
                        {
                            cout << "Graph drawn successfully" << endl;
                            vector<int> sortedVertices;
                            sortVerticesByDegree(degrees, sortedVertices);
                            cout << "Sorted vertices by degree in ascending order (User`s Graph): \n";
                            for (const auto& vertex : sortedVertices)
                            {
                                cout << "Vertex " << vertex + 1 << ": Degree " << degrees[vertex] << endl;
                            }
                            cout << "------------------------------------------------------------------ \n";
                            cout << "Press Enter to generate random graphs..." << endl;
                            cout << "------------------------------------------------------------------ \n";
                            cin.ignore(); // Ignore any remaining newline character from previous input
                            cin.get();
                            RenderWindow window1(VideoMode(800, 600), "Random Graph 1");
                            RenderWindow window2(VideoMode(800, 600), "Random Graph 2");
                            vector<vector<int>> adjacencyMatrix1 = generateRandomGraph(numVertices, numEdges);
                            shuffleVerticesWithSameEdges(adjacencyMatrix1); // Shuffle the vertices while preserving the edges
                                                        // Create the dots for the first random graph
                            vector<Vector2f> graph1Dots;
                            for (int i = 0; i < numVertices; ++i)
                            {
                                int x = uniform_int_distribution<>(DotSpacing, MaxX - DotSpacing)(gen);
                                int y = uniform_int_distribution<>(DotSpacing, MaxY - DotSpacing)(gen);
                                graph1Dots.push_back(Vector2f(x, y));
                            }

                            // Create the lines for the first random graph
                            vector<Line> graph1Lines;
                            for (int i = 0; i < numVertices; ++i)
                            {
                                for (int j = i + 1; j < numVertices; ++j)
                                {
                                    if (adjacencyMatrix1[i][j] == 1)
                                    {
                                        graph1Lines.push_back(Line(graph1Dots[i], graph1Dots[j], false));
                                    }
                                }
                            }

                            vector<int> degrees1 = calculateDegrees(adjacencyMatrix1);
                            vector<int> sortedVertices1;
                            sortVerticesByDegree(degrees1, sortedVertices1);
                            cout << "Sorted vertices by degree in ascending order (Random Graph 1):" << endl;
                            for (const auto& vertex : sortedVertices1) {
                                cout << "Vertex " << vertex + 1 << ": Degree " << degrees1[vertex] << endl;
                            }
                            cout << "------------------------------------------------------------------" << endl;

                            // Generate the second random graph with the same number of vertices and edges
                            vector<vector<int>> adjacencyMatrix2 = adjacencyMatrix1; // Start with the same adjacency matrix
                            shuffleVerticesWithSameEdges(adjacencyMatrix2); // Shuffle the vertices while preserving the edges
                            // Create the dots for the second random graph
                            vector<Vector2f> graph2Dots;
                            for (int i = 0; i < numVertices; ++i)
                            {
                                int x = uniform_int_distribution<>(DotSpacing, MaxX - DotSpacing)(gen);
                                int y = uniform_int_distribution<>(DotSpacing, MaxY - DotSpacing)(gen);
                                graph2Dots.push_back(Vector2f(x, y));
                            }
                            // Create the lines for the second random graph based on the shuffled adjacency matrix
                            vector<Line> graph2Lines;
                            for (int i = 0; i < numVertices; ++i)
                            {
                                for (int j = i + 1; j < numVertices; ++j)
                                {
                                    if (adjacencyMatrix2[i][j] == 1)
                                    {
                                        graph2Lines.push_back(Line(graph2Dots[i], graph2Dots[j], false));
                                    }
                                }
                            }
                            vector<int> degrees2 =
                                calculateDegrees(adjacencyMatrix2);
                            vector<int> sortedVertices2;
                            sortVerticesByDegree(degrees2, sortedVertices2);

                            cout << "Sorted vertices by degree in ascending order (Random Graph 2):" << endl;
                            for (const auto& vertex : sortedVertices2) {
                                cout << "Vertex " << vertex + 1 << ": Degree " << degrees2[vertex] << endl;
                            }
                            cout << "------------------------------------------------------------------" << endl;
                            // Check for isomorphism with the first random graph (graph1)
                            if (isIsomorphic(degrees, degrees1)) {
                                cout << "User-Graph is isomorphic to Random Graph 1." << endl;
                            }
                            else {
                                cout << "User-Graph is NOT isomorphic to Random Graph 1." << endl;
                            }

                            // Check for isomorphism with the second random graph (graph2)
                            if (isIsomorphic(degrees, degrees2)) {
                                cout << "User-Graph is isomorphic to Random Graph 2." << endl;
                            }
                            else {
                                cout << "User-Graph is NOT isomorphic to Random Graph 2." << endl;
                            }
                            while (window1.isOpen() && window2.isOpen()) {
                                Event event1, event2;
                                while (window1.pollEvent(event1)) {
                                    if (event1.type == Event::Closed)
                                        window1.close();
                                }
                                while (window2.pollEvent(event2)) {
                                    if (event2.type == Event::Closed)
                                        window2.close();
                                }
                                window1.clear();
                                window2.clear();
                                drawGraph(window1, graph1Dots, graph1Lines);
                                drawGraph(window2, graph2Dots, graph2Lines);
                                window1.display();
                                window2.display();
                            }
                            return 0;
                        }
                    }
                    else
                    {
                        // Stop dragging any dot
                        for (auto& dot : dots)
                        {
                            if (dot.isDragging)
                            {
                                dot.isDragging = false;
                                dot.shape.setFillColor(Color::Yellow); // Reset the dot's color
                            }
                        }
                    }
                }
                else if (event.type == Event::MouseMoved)
                {
                    Vector2f mousePos = Vector2f(Mouse::getPosition(window));
                    for (auto& dot : dots)
                    {
                        if (dot.isDragging)
                        {
                            dot.shape.setPosition(mousePos);
                            // Update the position of the label relative to the dot's position
                            dot.label.setPosition(dot.shape.getPosition() + Vector2f(5, 2));
                            break;
                        }
                    }
                }
            }
            if (drawingMode && isDrawing)
            {
                endPos = Vector2f(Mouse::getPosition(window));
            }
            window.clear();
            for (const auto& dot : dots)
            {
                window.draw(dot.shape);
                window.draw(dot.label); // Draw the number or alphabet label
                for (const auto& curve : dot.curves)
                {
                    window.draw(curve);
                }
            }
            for (const auto& edge : edges)
            {
                window.draw(edge.line);
                window.draw(edge.label); // Draw the edge label
            }
            if (drawingMode && isDrawing)
            {
                Vertex line[] =
                {
                    Vertex(startPos, Color::Cyan),
                    Vertex(endPos, Color::Cyan)
                };
                window.draw(line, 2, Lines);
            }
            Text instructionText;
            instructionText.setFont(font);
            instructionText.setString("INSTRUCTION:  Press the (SPACEBAR) to Switch to Move the Vertices\nor to Draw Edges. Press (L) and Click the Right Border of the Vertex\nto Create a Loop. Take Note That The Last Edge Shouldn't be a Loop.");
            instructionText.setCharacterSize(20);
            instructionText.setFillColor(Color::White);
            instructionText.setPosition(10, 10);
            window.draw(instructionText);
            window.display();
        }
    }
    return 0;
}


