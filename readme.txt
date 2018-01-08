This is a wikipedia graph search. Each article represents one node in the Graph, links on the page are represented as nodes.

We used a bag of words approach to determindes semantic relation between articles.

Overview of tested Wikipedia files:
128M  "kill-host.xml" 		
2,1M  "wikipedia-love.xml"
436K  "wikipedia-liebe.xml"

Execute:	-mpirun -machinefile ../machine_file ./main <filename>

This project was part of the Algorithm Engineering course at Friedrich Schiller University Jena.
http://theinf2.informatik.uni-jena.de/Lectures/Algorithm+Engineering.html
