add_library(engine OBJECT
    deck.cpp 
    evaluator.cpp 
    game.cpp 
    pot.cpp
    card.cpp
)


set(ALL_OBJECT_FILES
  ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:engine> 
  PARENT_SCOPE
)
