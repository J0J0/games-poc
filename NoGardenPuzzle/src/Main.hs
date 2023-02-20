module Main where
    
import Protolude hiding (State, rotate, lines)
import Graphics.Gloss hiding (Line)
import Graphics.Gloss.Interface.Pure.Game (Event(..))
import qualified Graphics.Gloss.Interface.Pure.Game as Ev (Key(..), MouseButton(..), KeyState(..))

import qualified Data.Array.IArray as A

(.:) :: (b -> c) -> (a1 -> a2 -> b) -> a1 -> a2 -> c
(.:) = (.).(.)

both :: (a -> b) -> (a,a) -> (b,b)
both f (x,y) = (f x, f y)

foldrMay1 :: (a -> a -> Maybe a) -> [a] -> Maybe a
foldrMay1 _ []     = Nothing
foldrMay1 _ [x]    = return x
foldrMay1 f (x:xs) = f x =<< foldrMay1 f xs

lightOrange :: Color
lightOrange = makeColorI 255 215 135 255


main :: IO ()
main = play win white 10 testState drawAll eventHandler'{-DBG-} (const identity)
  where
    width  = 700
    height = 500
    win = InWindow "No Garden Puzzle" (width,height) (1000,0)
    
testState = State b [] Nothing []
  where
    b' = A.listArray ((0,0),(6,5)) (repeat EmptyTile)
    b = b' A.// [((1,1), ObstacleTile), ((3,2), ObstacleTile)]
    --ls = [l1]
    --l1 = Line (Segment (6,0) West) [Straight, Straight, TurnRight, Straight, TurnLeft, TurnRight]
    --us = map (\ t -> (t, UsedTile)) [(6,0),(5,0),(4,0),(4,1),(4,2),(3,2)]
    --b  = b1 A.// us


data Tile = EmptyTile
          | UsedTile
          | ObstacleTile
          
type Coord = (Int,Int)
type Board = A.Array Coord Tile

data LineDir = Straight | TurnLeft | TurnRight
data CardinalDir = North | East | South | West
    deriving (Enum, Bounded)
data LineSegment = Segment Coord CardinalDir
data Line = Line { startSeg    :: LineSegment
                 , directions  :: [LineDir]
                 }
          
data State = State { board :: Board
                   , lines :: [Line]
                   , previewLine :: Maybe Line
                   , currentLine :: [Line]
                   }

data Orientation = Horizontal | Vertical


tileSize, spacing :: Float
tileSize = 40
spacing  = 1


toPoint :: Coord -> Point
toPoint = both (*. (tileSize + spacing))
  where
    x *. y = fromIntegral x * y

toCoord :: Point -> Coord
toCoord = both (\ z -> floor $ (z+tileSize/2)/(tileSize+spacing))

dirToAngle :: CardinalDir -> Float
dirToAngle = fromIntegral . (90*) . fromEnum

invertDir :: CardinalDir -> CardinalDir
invertDir North = South
invertDir East  = West
invertDir South = North
invertDir West  = East


isEmptyTile :: Tile -> Bool
isEmptyTile EmptyTile = True
isEmptyTile _         = False

boardMax :: Board -> (Int,Int)
boardMax = snd . A.bounds

boardDims :: Board -> (Int,Int)
boardDims = both (+1) . snd . A.bounds

boundaryCoords, boundaryCoordsFree :: Board -> [(CardinalDir, Coord)]
boundaryCoords = boundaryCoordsFilter (const True)
boundaryCoordsFree b = boundaryCoordsFilter (isEmptyTile . (b A.!)) b

-- filter based on nearest coordinate inside the board
boundaryCoordsFilter :: (Coord -> Bool) -> Board -> [(CardinalDir, Coord)]
boundaryCoordsFilter f b = bdy_n ++ bdy_e ++ bdy_s ++ bdy_w
  where
    (mx,my) = boardMax b
    bdy_w   = [(West,  (-1,  k)) | k <- [0..my], f (0, k)]
    bdy_e   = [(East,  (mx+1,k)) | k <- [0..my], f (mx,k)]
    bdy_s   = [(South, (k,  -1)) | k <- [0..mx], f (k, 0)]
    bdy_n   = [(North, (k,my+1)) | k <- [0..mx], f (k,my)]

startCoord :: Line -> Coord
startCoord l = case startSeg l of Segment c _ -> c

startDir :: Line -> CardinalDir
startDir l = case startSeg l of Segment _ dir -> dir

orientationOf :: CardinalDir -> Orientation
orientationOf North = Vertical
orientationOf East  = Horizontal
orientationOf South = Vertical
orientationOf West  = Horizontal

emptyLine :: Coord -> CardinalDir -> Line
emptyLine c dir = Line { startSeg   = Segment c dir
                       , directions = [] }

guardEmptyLine :: Line -> Maybe Line
guardEmptyLine l = case directions l of
    [] -> Nothing
    _  -> Just l

isEmptyLine :: Line -> Bool
isEmptyLine = isNothing . guardEmptyLine

--------------------

boardCenter :: Board -> Point
boardCenter = both (\ z -> fromIntegral z*(tileSize+spacing)/2) . boardMax

globalTranslate :: State -> Picture -> Picture
globalTranslate st = translate dx dy
  where
    bc = boardCenter (board st)
    (dx, dy) = both negate bc

eventTranslate :: State -> Event -> Event
eventTranslate st ev =
    let bc = boardCenter (board st)
    in case ev of
        EventKey k ks mod pt -> EventKey k ks mod (pt +. bc)
        EventMotion pt       -> EventMotion (pt +. bc)
        ev                   -> ev
  where
    (x,y) +. (a,b) = (x+a, y+b)

--------------------

drawAt :: Coord -> Picture -> Picture
drawAt = uncurry translate . toPoint


drawAll :: State -> Picture
drawAll st = globalTranslate st $
    pictures [ drawBoard (board st)
             , drawLines (lines st)
             , maybe Blank drawCurrentLine (joinRevLines . currentLine $ st)
             , maybe Blank drawPreviewLine (previewLine st)
             , case (previewLine st, currentLine st) of
                 (Nothing, []) -> drawStartHints st
                 _             -> Blank
             , case (previewLine st, currentLine st) of
                 (Nothing, (_:_)) -> drawContinueHints st
                 _                -> Blank
             , winningMessage st
             ]

drawBoard :: Board -> Picture
drawBoard b = pictures $ map (uncurry drawTile) $ A.assocs b

drawTile :: Coord -> Tile -> Picture
drawTile coord tile = drawAt coord $ color_for tile $ rectangleSolid tS tS
  where
    tS = tileSize
    color_for ObstacleTile = color black
    color_for _            = color (greyN 0.8)

drawLines :: [Line] -> Picture
drawLines = pictures . map (drawLine (greyN 0.5))

drawLine :: Color -> Line -> Picture
drawLine c l =
    drawAt (startCoord l)
    $ color c
    $ rotate (dirToAngle (startDir l))
    $ drawLineSegments (directions l)

drawCurrentLine :: Line -> Picture
drawCurrentLine = drawLine (greyN 0.35)

drawPreviewLine :: Line -> Picture
drawPreviewLine = drawLine lightOrange

drawLineSegments :: [LineDir] -> Picture
drawLineSegments []       = Blank
drawLineSegments (ld:lds) = drawLineSegment ld <> transform ld (drawLineSegments lds)
  where
    transform :: LineDir -> Picture -> Picture
    transform Straight  = translate 0 (tileSize+spacing)
    transform TurnLeft  = turn_trafo (-1)
    transform TurnRight = turn_trafo   1
    turn_trafo sign =
        translate (sign * (tileSize+spacing)) 0 . rotate (sign * 90)

drawLineSegment :: LineDir -> Picture
drawLineSegment Straight  = rectangleSolid (tileSize/3) tileSize
drawLineSegment ld        = case ld of TurnLeft  -> draw_turn (-1)  0
                                       TurnRight -> draw_turn   1  90
  where
    th = tileSize/2
    draw_turn sign phi =
        translate (sign*th) (-th) $ thickArc phi (phi+90) th (tileSize/3)

drawStartHints :: State -> Picture
drawStartHints st =
    pictures $ map (uncurry . flip $ drawArrow lightOrange) boundary
  where
    (mx,my)  = boardMax (board st)
    boundary = map (first invertDir) $ boundaryCoordsFree (board st)

drawContinueHints :: State -> Picture
drawContinueHints st = case currentLine st of
    []    -> Blank
    (l:_) -> maybe Blank go $ lastMay (lineSegments l)
  where
    go = pictures . map (uncurry $ drawArrow lightOrange) . filter legal_move . cont_coord
    cont_coord (Segment (x,y) dir) = case orientationOf dir of
        Vertical   -> [((x-1,y), West),  ((x+1,y), East)]
        Horizontal -> [((x,y-1), South), ((x,y+1), North)]
    legal_move (c@(x,y), _)
        | x < 0 || y < 0 || x > mx || y > my  = True
        | isEmptyTile (board st A.! c)        = True
        | otherwise                           = False
    (mx,my) = boardMax (board st)

drawArrow :: Color -> Coord -> CardinalDir -> Picture
drawArrow c coord dir =
    drawAt coord
    $ color c
    $ rotate (dirToAngle dir)
    $ pictures [ translate 0 (0-0.05*tS) $ rectangleSolid (0.2*tS) (0.35*tS)
               , polygon [(0.25*tS,0), (0,0.4*tS), (-0.25*tS,0)]
               ]
  where
    tS = tileSize

winningMessage :: State -> Picture
winningMessage st = case currentLine st of
    (_:_) -> Blank
    []    -> if any isEmptyTile (board st)
             then Blank
             else drawAt (0-3,0) $ color green $ scale 0.7 0.7 $ Text "You won!"

--------------------

eventHandler' :: Event -> State -> State
eventHandler' = eventHandler
--eventHandler' ev st = traceShow (eventTranslate st ev) $ eventHandler ev st

eventHandler :: Event -> State -> State
eventHandler ev st =
    case eventTranslate st ev of
        EventMotion pt -> hover (toCoord pt) st
        EventKey (Ev.MouseButton Ev.LeftButton)  Ev.Up _ _ -> addLine st
        EventKey (Ev.MouseButton Ev.RightButton) Ev.Up _ _ -> cancelCurrentLine st
        EventKey (Ev.Char 'r')                   Ev.Up _ _ -> resetBoard st
        _ -> st

--------------------

hover :: Coord -> State -> State
hover (x,y) st = st { previewLine = (if null (currentLine st)
                          then startLine mx my x y
                          else continueLine st x y)
                          <&> extendLine st
--                          >>= guardEmptyLine
                    }
  where
      (mx,my) = boardMax (board st)

startLine :: Int -> Int -> Int -> Int -> Maybe Line
startLine mx my x y = case (x,mx-x,y,my-y) of
    (-1,_,_,_) | withinY -> mk_line (0,y)  East
    (_,-1,_,_) | withinY -> mk_line (mx,y) West
    (_,_,-1,_) | withinX -> mk_line (x,0)  North
    (_,_,_,-1) | withinX -> mk_line (x,my) South
    _                    -> Nothing
  where
    mk_line = Just .: emptyLine
    withinX = 0 <= x && x <= mx
    withinY = 0 <= y && y <= my

continueLine :: State -> Int -> Int -> Maybe Line
continueLine st x y =
    listToMaybe (currentLine st)
    >>= (lastMay . lineSegments)
    >>= \ (Segment (lx,ly) dir) ->
        case (x-lx, y-ly, orientationOf dir) of
            (-1, 0, Vertical)   -> mk_line (lx-1,ly) West
            ( 1, 0, Vertical)   -> mk_line (lx+1,ly) East
            ( 0, 1, Horizontal) -> mk_line (lx,ly+1) North
            ( 0,-1, Horizontal) -> mk_line (lx,ly-1) South
            _                   -> Nothing
  where
    mk_line = Just .: emptyLine

-- buggy for non-empty line!
extendLine :: State -> Line -> Line
extendLine st l =
    l { directions = directions l ++ extendStraight st seg }
  where
    seg = lastDef (startSeg l) $ lineSegments l


extendStraight :: State -> LineSegment -> [LineDir]
extendStraight st (Segment c dir) = Straight <$ takeWhile isEmptyTile tiles
  where
    tiles    = (board st A.!) <$> take max_num (iterate step c)
    (x,y)    = c
    (xd, yd) = boardDims (board st)
    (max_num, step) = case dir of
        North -> (yd-y, second (+1)        )
        East  -> (xd-x, first  (+1)        )
        South -> (y+1,  second (subtract 1))
        West  -> (x+1,  first  (subtract 1))

--------------------

addLine :: State -> State
addLine st = case previewLine st of
    Nothing -> st
    Just l  -> if lineComplete st l
               then State { board = board st A.// gen_updates l
                          , lines = (maybeToList $ joinRevLines (l : currentLine st)) ++ lines st
                          , currentLine = []
                          , previewLine = Nothing
                          }
               else if isEmptyLine l then st
                    else st { board = board st A.// gen_updates l
                            , currentLine = l : currentLine st
                            , previewLine = Nothing
                            }
  where
    gen_updates = map (\ t -> (t, UsedTile)) . lineCoordinates

lineComplete :: State -> Line -> Bool
lineComplete st l =
    case lastDef (startSeg l, 1) (lineSegments l `zip` repeat 0) of
        (Segment (x,y) dir, t) ->
            case (x+t, mx-x+t, y+t, my-y+t, dir) of
                (0,_,_,_, West)  -> True
                (_,0,_,_, East)  -> True
                (_,_,0,_, South) -> True
                (_,_,_,0, North) -> True
                _                -> False
  where
    (mx,my) = boardMax (board st)

joinLines :: [Line] -> Maybe Line
joinLines = foldrMay1 joinTwoLines

joinRevLines :: [Line] -> Maybe Line
joinRevLines = joinLines . reverse

-- first line should be non-empty
joinTwoLines :: Line -> Line -> Maybe Line
joinTwoLines l1 l2 = do
    Segment (x1,y1) dir1 <- lastMay (lineSegments l1)
    let Segment (x2,y2) dir2 = startSeg l2
    joinDir <- case (dir1, x2-x1, y2-y1, dir2) of
        (North,  0, 1, North) -> Just Straight
        (North,  1, 0, East)  -> Just TurnRight
        (North, -1, 0, West)  -> Just TurnLeft
        --
        (East,   1, 0, East)  -> Just Straight
        (East,   0, 1, North) -> Just TurnLeft
        (East,   0,-1, South) -> Just TurnRight
        --
        (South,  0,-1, South) -> Just Straight
        (South,  1, 0, East)  -> Just TurnLeft
        (South, -1, 0, West)  -> Just TurnRight
        --
        (West,  -1, 0, West)  -> Just Straight
        (West,   0, 1, North) -> Just TurnRight
        (West,   0,-1, South) -> Just TurnLeft
        --
        _ -> Nothing
    (d1',_) <- unsnoc (directions l1)
    return $ l1 { directions = d1' ++ [joinDir] ++ directions l2 }

--------------------

cancelCurrentLine :: State -> State
cancelCurrentLine st = case currentLine st of
    [] -> st
    ls -> st { board = board st A.// gen_updates ls
             , previewLine = Nothing
             , currentLine = []
             }
  where
    gen_updates = map (\ t -> (t, EmptyTile)) . concatMap lineCoordinates

--------------------

resetBoard :: State -> State
resetBoard st = State { board = reset_tile `A.amap` board st
                      , lines = []
                      , previewLine = Nothing
                      , currentLine = []
                      }
  where
    reset_tile UsedTile = EmptyTile
    reset_tile tile     = tile

--------------------

lineCoordinates :: Line -> [Coord]
lineCoordinates = map (\ (Segment coord _) -> coord) . lineSegments

lineSegments :: Line -> [LineSegment]
lineSegments l = case directions l of
    []     -> []
    (d:ds) -> startSeg l : lineSegments l'
      where
        l' = Line { startSeg = Segment c' o'
                  , directions = ds
                  }
        c' = step (startCoord l)
        (step, o') = case d of
            Straight  -> (caseDir up right down left, startDir l)
            TurnLeft  -> (caseDir left up right down, turnL        )
            TurnRight -> (caseDir right down left up, turnR        )
        caseDir n e s w =
            case startDir l of { North -> n; East  -> e;
                                 South -> s; West  -> w }
        up    = second (+1)
        right = first  (+1)
        down  = second (subtract 1)
        left  = first  (subtract 1)
        turnL = caseDir West North East South
        turnR = caseDir East South West North
