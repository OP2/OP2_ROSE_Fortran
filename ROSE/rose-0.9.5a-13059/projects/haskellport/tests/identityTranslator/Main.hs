module Main where

import ROSE
import System

main = do
	project <- frontend =<< getArgs
	exitWith =<< backend project
