from contextlib import redirect_stdout
import importlib.util
import io
from pathlib import Path
import unittest


SCRIPT = Path(__file__).with_name("MoveVariableDeclarations.py")
SPEC = importlib.util.spec_from_file_location("move_declarations", SCRIPT)
move = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(move)


def categories(source):
    return {finding.name: finding.category
            for finding in move.analyze(source.splitlines(keepends=True))}


class MoveVariableDeclarationsTest(unittest.TestCase):
    def test_moves_each_movable_name_from_comma_declaration(self):
        source = "{\n  int first, second;\n\n  first = one();\n  second = two();\n}\n"
        expected = "{\n  int first = one();\n  int second = two();\n}\n"
        self.assertEqual(move.rewrite(source, "test.c", report=False), expected)

    def test_declaration_comment_is_not_duplicated_when_split(self):
        source = "{\n  int first, second; // values\n  first = 1;\n  second = 2;\n}\n"
        expected = "{\n  int first = 1; // values\n  int second = 2;\n}\n"
        self.assertEqual(move.rewrite(source, "test.c", report=False), expected)

    def test_keeps_nonmovable_name_in_comma_declaration(self):
        source = "{\n  int used, safe;\n\n  consume(used);\n  used = 1;\n  safe = 2;\n}\n"
        expected = "{\n  int used;\n\n  consume(used);\n  used = 1;\n  int safe = 2;\n}\n"
        self.assertEqual(categories(source),
                         {"used": "used before assignment", "safe": "movable"})
        self.assertEqual(move.rewrite(source, "test.c", report=False), expected)

    def test_rejects_rhs_use_at_first_assignment(self):
        source = "{\n  int value;\n\n  value = value + 1;\n}\n"
        self.assertEqual(categories(source), {"value": "used before assignment"})

    def test_categorizes_multiple_and_macro_assignments(self):
        source = (
            "{\n  int twice, character;\n\n"
            "  twice = 1;\n  twice = 2;\n"
            "  GETCHAR(character, pointer);\n}\n"
        )
        self.assertEqual(categories(source),
                         {"twice": "multiple assignment",
                          "character": "macro-assigned"})
        self.assertEqual(move.rewrite(source, "test.c", report=False), source)

    def test_first_relevant_event_wins_over_later_macro_write(self):
        direct_first = (
            "{\n  int value;\n\n"
            "  value = 1;\n  GETCHAR(value, pointer);\n}\n"
        )
        use_first = (
            "{\n  int value;\n\n"
            "  consume(value);\n  GETCHAR(value, pointer);\n}\n"
        )
        self.assertEqual(categories(direct_first), {"value": "movable"})
        self.assertEqual(categories(use_first),
                         {"value": "used before assignment"})

    def test_tracks_conditional_preprocessor_branches(self):
        source = "{\n  int value;\n\n#if FEATURE\n  value = 1;\n#endif\n}\n"
        self.assertEqual(categories(source), {"value": "conditional"})

        source = (
            "{\n#if FEATURE\n  int value;\n#else\n"
            "  value = 1;\n#endif\n}\n"
        )
        self.assertEqual(categories(source), {"value": "conditional"})

        source = (
            "{\n#if FEATURE\n  int value;\n  value = 1;\n#endif\n}\n"
        )
        self.assertEqual(categories(source), {"value": "movable"})

    def test_categorizes_label_and_goto_sensitive_spans(self):
        source = "{\n  int value;\n\n  goto done;\ndone:\n  value = 1;\n}\n"
        self.assertEqual(categories(source), {"value": "label-goto-sensitive"})

        source = "{\n  int value;\n\nlabel:\n  value = 1;\n}\n"
        self.assertEqual(categories(source), {"value": "label-goto-sensitive"})

    def test_reports_never_assigned_and_ignores_non_scope_start(self):
        source = "{\n  int never;\n\n  work();\n  int late;\n  late = 1;\n}\n"
        self.assertEqual(categories(source), {"never": "never directly assigned"})

    def test_grouped_report_has_manual_queue_counts_and_exact_proportion(self):
        source = (
            "{\n  int safe, used, never;\n\n"
            "  safe = 1;\n  consume(used);\n}\n"
        )
        findings = [("sample.c", finding)
                    for finding in move.analyze(source.splitlines(keepends=True))]
        output = io.StringIO()
        with redirect_stdout(output):
            move.print_report(findings)
        report = output.getvalue()
        self.assertIn("AUTOMATIC MOVE QUEUE\nmovable (1)", report)
        self.assertIn("MANUAL QUEUE", report)
        self.assertIn("used before assignment (1)", report)
        self.assertIn("never directly assigned (1)", report)
        self.assertIn("  sample.c:2: used", report)
        self.assertIn(
            "moved/(moved + remaining scope-start uninitialized variables): "
            "1/(1+2) = 1/3",
            report,
        )


if __name__ == "__main__":
    unittest.main()
