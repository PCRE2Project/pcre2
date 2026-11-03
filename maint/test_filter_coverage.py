import contextlib
import io
from pathlib import Path
import tempfile
import unittest

from FilterCoverage import filter_lcov


class FilterCoverageTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.source = Path(self.directory.name) / "source.c"
        self.source.write_text(
            "int f(void)\n"
            "{\n"
            "  if (condition)\n"
            "    return 1;\n"
            "  return 0; /* LCOV_EXCL_LINE */\n"
            "}\n",
            encoding="utf-8",
        )

    def filter(self, records):
        output = io.StringIO()
        diagnostics = io.StringIO()
        with contextlib.redirect_stderr(diagnostics):
            filter_lcov(
                io.StringIO(f"SF:{self.source}\n{records}end_of_record\n"),
                output,
            )
        return output.getvalue().splitlines(), diagnostics.getvalue()

    def test_exclusions_and_summaries(self):
        output, diagnostics = self.filter(
            "FN:1,f\nFN:5,excluded\nFNDA:2,f\nFNDA:0,excluded\n"
            "DA:3,2\nDA:4,1\nDA:5,0\nDA:6,2\n"
            "BRDA:3,0,0,1\nBRDA:3,0,1,1\n"
            "BRDA:5,0,0,0\nBRDA:5,0,1,0\n"
            "FNF:2\nFNH:1\nLF:4\nLH:3\nBRF:4\nBRH:2\n"
        )
        self.assertNotIn("FN:5,excluded", output)
        self.assertNotIn("FNDA:0,excluded", output)
        self.assertNotIn("DA:5,0", output)
        self.assertNotIn("DA:6,2", output)
        self.assertNotIn("BRDA:5,0,0,0", output)
        for record in ("FNF:1", "FNH:1", "LF:2", "LH:2", "BRF:2", "BRH:2"):
            self.assertIn(record, output)
        self.assertEqual(diagnostics, "")

    def test_llvm_summaries_are_recomputed_from_records(self):
        output, diagnostics = self.filter(
            "FN:1,f_8\nFN:1,f_16\nFNDA:2,f_8\nFNDA:0,f_16\n"
            "DA:3,2\nDA:4,0\nBRDA:3,0,0,2\nBRDA:3,0,1,0\n"
            "FNF:1\nFNH:1\nLF:0\nLH:0\nBRF:1\nBRH:0\n"
        )
        for record in ("FNF:2", "FNH:1", "LF:2", "LH:1", "BRF:2", "BRH:1"):
            self.assertIn(record, output)
        self.assertEqual(diagnostics, "")

    def test_orphan_branches_get_a_line_count_lower_bound(self):
        output, diagnostics = self.filter(
            "BRDA:3,0,0,2\nBRDA:3,0,1,7\n"
            "BRDA:3,1,2,1\nBRDA:3,1,3,3\n"
            "LF:0\nLH:0\nBRF:4\nBRH:4\n"
        )
        self.assertIn("DA:3,9", output)
        self.assertIn("LF:1", output)
        self.assertIn("LH:1", output)
        self.assertEqual(sum(line.startswith("BRDA:") for line in output), 4)
        self.assertIn("inferred", diagnostics)
        self.assertIn(f"{self.source}:3", diagnostics)

    def test_unevaluated_orphan_branches(self):
        output, diagnostics = self.filter(
            "BRDA:3,0,0,-\nBRDA:3,0,1,-\n"
            "LF:0\nLH:0\nBRF:2\nBRH:0\n"
        )
        self.assertIn("DA:3,0", output)
        self.assertIn("LF:1", output)
        self.assertIn("LH:0", output)
        self.assertIn("BRDA:3,0,0,-", output)
        self.assertIn("inferred", diagnostics)

    def test_switch_case_counts_are_not_rewritten(self):
        records = "DA:3,0\nBRDA:3,0,0,0\nBRDA:3,0,1,37\n"
        output, diagnostics = self.filter(records + "LF:1\nLH:0\nBRF:2\nBRH:1\n")
        for record in records.splitlines():
            self.assertIn(record, output)
        self.assertIn("LH:0", output)
        self.assertEqual(diagnostics, "")

    def test_short_circuit_counts_are_not_rewritten(self):
        records = "DA:3,37\nBRDA:3,0,0,-\nBRDA:3,0,1,-\n"
        output, diagnostics = self.filter(records + "LF:1\nLH:1\nBRF:2\nBRH:0\n")
        for record in records.splitlines():
            self.assertIn(record, output)
        self.assertIn("BRH:0", output)
        self.assertEqual(diagnostics, "")

    def test_function_end_lines_are_preserved(self):
        output, diagnostics = self.filter(
            "FN:1,6,f\nFN:5,6,excluded\nFNDA:2,f\nFNDA:0,excluded\n"
            "FNF:2\nFNH:1\n"
        )
        self.assertIn("FN:1,6,f", output)
        self.assertNotIn("FN:5,6,excluded", output)
        self.assertNotIn("FNDA:0,excluded", output)
        self.assertIn("FNF:1", output)
        self.assertEqual(diagnostics, "")

    def test_duplicate_line_records_count_as_one_coverpoint(self):
        output, diagnostics = self.filter("DA:3,0\nDA:3,2\nLF:1\nLH:1\n")
        self.assertIn("DA:3,0", output)
        self.assertIn("DA:3,2", output)
        self.assertIn("LF:1", output)
        self.assertIn("LH:1", output)
        self.assertEqual(diagnostics, "")

    def test_excluded_orphan_branches_do_not_create_lines(self):
        output, diagnostics = self.filter(
            "BRDA:5,0,0,4\nBRDA:5,0,1,0\nLF:0\nLH:0\nBRF:2\nBRH:1\n"
        )
        self.assertFalse(any(line.startswith(("DA:", "BRDA:")) for line in output))
        self.assertIn("LF:0", output)
        self.assertEqual(diagnostics, "")

    def test_large_counts_are_preserved(self):
        output, diagnostics = self.filter(
            "DA:3,8589934592\nBRDA:3,0,0,8589934592\nBRDA:3,0,1,0\n"
            "LF:1\nLH:1\nBRF:2\nBRH:1\n"
        )
        self.assertIn("DA:3,8589934592", output)
        self.assertIn("BRDA:3,0,0,8589934592", output)
        self.assertEqual(diagnostics, "")

    def test_multiple_source_records_and_idempotence(self):
        source = self.source.with_name("second.c")
        source.write_text("return 0;\n", encoding="utf-8")
        original = (
            f"TN:tests\nSF:{self.source}\n"
            "BRDA:3,0,0,2\nBRDA:3,0,1,7\nLF:0\nLH:0\nend_of_record\n"
            f"SF:{source}\nDA:1,0\nLF:1\nLH:0\nend_of_record\n"
        )
        first = io.StringIO()
        with contextlib.redirect_stderr(io.StringIO()):
            filter_lcov(io.StringIO(original), first)
        self.assertIn("DA:3,9\nend_of_record\n", first.getvalue())
        second = io.StringIO()
        diagnostics = io.StringIO()
        with contextlib.redirect_stderr(diagnostics):
            filter_lcov(io.StringIO(first.getvalue()), second)
        self.assertEqual(first.getvalue(), second.getvalue())
        self.assertEqual(diagnostics.getvalue(), "")

    def test_exclusion_blocks_and_preprocessor_lines(self):
        self.source.write_text(
            "#if FEATURE\n"
            "if (condition) /* LCOV_EXCL_START */\n"
            "  return 1;\n"
            "/* LCOV_EXCL_STOP */\n"
            "#endif\n"
            "return 0;\n",
            encoding="utf-8",
        )
        output, diagnostics = self.filter(
            "".join(f"DA:{line},1\nBRDA:{line},0,0,1\n" for line in range(1, 7))
            + "LF:6\nLH:6\nBRF:6\nBRH:6\n"
        )
        self.assertEqual([line for line in output if line.startswith("DA:")], ["DA:6,1"])
        self.assertEqual([line for line in output if line.startswith("BRDA:")], ["BRDA:6,0,0,1"])
        for record in ("LF:1", "LH:1", "BRF:1", "BRH:1"):
            self.assertIn(record, output)
        self.assertEqual(diagnostics, "")


if __name__ == "__main__":
    unittest.main()
