## TODO
 - Create plan for Perun rework [x]


Goal: Reach Top 200 in CCRL
----

# Plan

Goal Steps:

1\. Create Working UCI compatible engine ( a good engine )

1.1. Create engine with a good base for improvement \
1.2. Create framework for improving

1.1.1 Plan the whole engine before creating components \
1.1.2 Create the componens that are general and able to change over time \
1.2.1 Create framework for assessing the engine power \
1.2.2 Plan Engine improvements still considering future changes \
1.2.3 Improve the engine according to plans continuusly until success

merged steps:
1. Plan the whole engine before creating components
2. Create the components that are general and able to change over time
3. Create framework for assessing the engine power
4. Plan Engine improvements still considering future changes
5. Improve the engine according to plans continuusly until success

### Step 1. (Plan the whole engine before creating components)
 - Choose algorithms and approaches
   - Divide the Engine into logical components which can be changed or replaced without affecting the whole engine
   - Research top engins solutions and other algorithms. Write down pros, cons, implementation difficulty, interface compatibilities and possible future improvements ( all per component )
   - For each component, if it has possible subcomponents, perform the same analysis for them all the way down util there are no algoritmics options to choose
   - For higher level components create graphs or pseudocode of subcomponents dependencies or ordering
 - Create logical links between components
   - create a graph represinting all level components dependencies for interface planning
 - Specify most important parts data structures and high level interfaces for blocks
   - based on dependency graph create interfaces pseudocode with analysys of load
   - plan the data structures based on load and needs of the engine. Perform analysis of target system types and plan the structures for different system flawors

### Step 2. (Create the components that are general and able to change over time)
 - Design the detailed level interfaces (classes / methods)
   - create component header files
   - create documentation with complexity analysis
 - Assert design pattern correctnes and compilation optimizations to make sure engine will not slow because of scalability patterns
   - Based on the documentation perform first tests (code analysis) and create classes/methods/components risk analysis based on that
 - Implement the components
   - Create build system for different system flawors and build options
   - implement components starting from lowest level components
 - Write tests to assert the base level code works correct to avoid problems in the future
   - create unit and integration test framework
   - write tests for implemented components

### Step 3. (Create framework for assessing the engine power)
 - Research available solutions and tech stack for the framework
   - Research puzzle strenght determining
   - Research tournament strength determining
   - Research self play strength determining
 - Create the framework
   - Create framework for overal strength assessing
   - Create framework part for assessing other engin aspects separately like speed or static evaluation
 - Assert correctness on other engine
   - Assert correctness on 3 most played engines on CCRL

### Step 3.5
 - Upload the first official version of the engine

### Step 4. (Plan Engine improvements still considering future changes)
 - Find out the bottlenecks and weak sides of the engine through framework testing or other analysis
 - Create Working plan and definition of done estimates for engin power

### Step 5. (Improve the engine according to plans continuusly until success)
 - After each improvement create a release and extensive engine power tests
 - Decide if the improvement is worthy to be integrated into main engine
 - Repeat Step 4 and 5 util success

> Repeat Step 4 Until 3000 elo

## Success