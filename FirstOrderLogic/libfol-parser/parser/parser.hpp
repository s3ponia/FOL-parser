#pragma once

#include <details/utils/utility.hpp>
#include <fstream>
#include <libfol-parser/lexer/lexer.hpp>
#include <libfol-parser/parser/exceptions.hpp>
#include <libfol-parser/parser/types.hpp>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <variant>

namespace fol::parser {

UnaryFormula ParseFolFormula(lexer::LexemeGenerator::iterator &iterator);
Term ParseTerm(lexer::LexemeGenerator::iterator &iterator);
TermList ParseTermList(lexer::LexemeGenerator::iterator &iterator);
DisjunctionFormula ParseDisjunctionFormula(
    lexer::LexemeGenerator::iterator &iterator);
DisjunctionPrimeFormula ParseDisjunctionPrimeFormula(
    lexer::LexemeGenerator::iterator &iterator);
ConjunctionFormula ParseConjuctionFormula(
    lexer::LexemeGenerator::iterator &iterator);
ConjunctionPrimeFormula ParseConjuctionPrimeFormula(
    lexer::LexemeGenerator::iterator &iterator);

TermListPrime ParseTermListPrime(lexer::LexemeGenerator::iterator &iterator);
TermList ParseTermList(lexer::LexemeGenerator::iterator &iterator);

ImplicationFormula ParseImplicationFormula(
    lexer::LexemeGenerator::iterator &iterator);

FolFormula Parse(lexer::LexemeGenerator generator);

}  // namespace fol::parser

