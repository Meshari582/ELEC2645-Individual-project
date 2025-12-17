// Calculator modules and shared safe input helpers.
// Design notes:
// All user input is read using fgets() and parsed using strtol()/strtod().
// Parsing is strict: inputs such as "12abc" are rejected.
// Safety/domain checks are used to avoid divide-by-zero and invalid maths.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <stdarg.h>   
#include "funcs.h"

static const char *LOG_FILE = "eee_log.txt";

int log_line(const char *line)
{
    FILE *fp = fopen(LOG_FILE, "a");
    if (!fp) return 0;
    fprintf(fp, "%s\n", line);
    fclose(fp);
    return 1;
}

void view_log(void)
{
    FILE *fp = fopen(LOG_FILE, "r");
    if (!fp) {
        printf("\n--- Saved Log ---\nNo saved calculations yet.\n");
        return;
    }

    printf("\n--- Saved Log ---\n");
    char buf[256];
    while (fgets(buf, sizeof buf, fp)) {
        fputs(buf, stdout);
    }
    fclose(fp);
}

// Logs a formatted line (keeps calculator working even if logging fails)
static void log_printf(const char *fmt, ...)
{
    char line[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(line, sizeof line, fmt, ap);
    va_end(ap);

    log_line(line);
}

#define PI 3.14159265358979323846

// ============================= INPUT HELPERS =============================

// Removes trailing newline characters (\n or \r\n) from a buffer read by fgets().
static void trim_newline(char *s)
{
    if (!s) return;
    s[strcspn(s, "\r\n")] = '\0';
}

// Prints a prompt (if provided) and reads a full line into buf.
// Returns 1 on success, 0 on EOF/input failure.
static int read_line(const char *prompt, char *buf, size_t size)
{
    if (prompt) printf("%s", prompt);
    if (!fgets(buf, size, stdin)) return 0;
    trim_newline(buf);
    return 1;
}

// Parses a long integer using strtol with strict validation.
// Rejects trailing junk (allows only trailing spaces/tabs).
static int parse_long(const char *s, long *out, int base)
{
    if (!s || !*s) return 0;

    errno = 0;
    char *end = NULL;
    long v = strtol(s, &end, base);

    if (errno == ERANGE) return 0;
    if (end == s) return 0;

    // Allow trailing whitespace only.
    while (*end == ' ' || *end == '\t') end++;
    if (*end != '\0') return 0;

    *out = v;
    return 1;
}

// Parses a double using strtod with strict validation.
// Rejects trailing junk (allows only trailing spaces/tabs).
static int parse_double(const char *s, double *out)
{
    if (!s || !*s) return 0;

    errno = 0;
    char *end = NULL;
    double v = strtod(s, &end);

    if (errno == ERANGE) return 0;
    if (end == s) return 0;

    // Allow trailing whitespace only.
    while (*end == ' ' || *end == '\t') end++;
    if (*end != '\0') return 0;

    *out = v;
    return 1;
}

// Re-prompts until the user enters a valid integer.
static int read_int(const char *prompt, int *out)
{
    char buf[128];

    for (;;) {
        if (!read_line(prompt, buf, sizeof buf)) return 0;

        long v;
        if (parse_long(buf, &v, 10)) {
            *out = (int)v;
            return 1;
        }
        printf("Invalid integer. Try again.\n");
    }
}

// Re-prompts until the user enters a valid floating-point number.
static int read_double(const char *prompt, double *out)
{
    char buf[128];

    for (;;) {
        if (!read_line(prompt, buf, sizeof buf)) return 0;

        double v;
        if (parse_double(buf, &v)) {
            *out = v;
            return 1;
        }
        printf("Invalid number. Try again.\n");
    }
}

// --------------------SAFETY HELPERS--------------------

// Performs safe division and rejects zero/near-zero denominators to avoid Inf/NaN.
// Returns 1 on success, 0 if denominator is too small.
static int safe_divide(double num, double den, double *out)
{
    const double eps = 1e-12; // treat very small denominators as zero

    if (out) *out = 0.0;
    if (fabs(den) < eps) return 0;

    if (out) *out = num / den;
    return 1;
}

// ------------------ 1) VOLTAGE DIVIDER -----------------------

void menu_item_1(void)
{
    printf("\n--- Voltage Divider ---\n");
    printf("Solve:\n");
    printf("1) Vout given Vin, R1, R2\n");
    printf("2) Vin  given Vout, R1, R2\n");
    printf("3) R1   given Vin, Vout, R2\n");
    printf("4) R2   given Vin, Vout, R1\n");

    int mode;
    if (!read_int("Select: ", &mode)) return;

    if (mode == 1) {
        // Vout = Vin * R2 / (R1 + R2)
        double Vin, R1, R2;
        if (!read_double("Vin (V): ", &Vin)) return;
        if (!read_double("R1 (ohms): ", &R1)) return;
        if (!read_double("R2 (ohms): ", &R2)) return;

        double ratio;
        if (!safe_divide(R2, (R1 + R2), &ratio)) {
            printf("Error: R1 + R2 cannot be zero (or near zero).\n");
            return;
        }
        double Vout = Vin * ratio;
        printf("Vout = %.6f V\n", Vout);

        log_printf("Voltage Divider (Vout): Vin=%.6f V, R1=%.6f ohm, R2=%.6f ohm -> Vout=%.6f V",
                   Vin, R1, R2, Vout);
    }
    else if (mode == 2) {
        // Vin = Vout * (R1 + R2) / R2
        double Vout, R1, R2;
        if (!read_double("Vout (V): ", &Vout)) return;
        if (!read_double("R1 (ohms): ", &R1)) return;
        if (!read_double("R2 (ohms): ", &R2)) return;

        double frac;
        if (!safe_divide((R1 + R2), R2, &frac)) {
            printf("Error: R2 cannot be zero (or near zero).\n");
            return;
        }
        double Vin_ans = Vout * frac;
        printf("Vin = %.6f V\n", Vin_ans);

        log_printf("Voltage Divider (Vin): Vout=%.6f V, R1=%.6f ohm, R2=%.6f ohm -> Vin=%.6f V",
                   Vout, R1, R2, Vin_ans);
    }
    else if (mode == 3) {
        // R1 = R2 * (Vin/Vout - 1)
        double Vin, Vout, R2;
        if (!read_double("Vin (V): ", &Vin)) return;
        if (!read_double("Vout (V): ", &Vout)) return;
        if (!read_double("R2 (ohms): ", &R2)) return;

        double Vin_over_Vout;
        if (!safe_divide(Vin, Vout, &Vin_over_Vout)) {
            printf("Error: Vout cannot be zero (or near zero).\n");
            return;
        }
        double R1_ans = R2 * (Vin_over_Vout - 1.0);
        printf("R1 = %.6f ohms\n", R1_ans);

        log_printf("Voltage Divider (R1): Vin=%.6f V, Vout=%.6f V, R2=%.6f ohm -> R1=%.6f ohm",
                   Vin, Vout, R2, R1_ans);
    }
    else if (mode == 4) {
        // R2 = R1 * Vout / (Vin - Vout)
        double Vin, Vout, R1;
        if (!read_double("Vin (V): ", &Vin)) return;
        if (!read_double("Vout (V): ", &Vout)) return;
        if (!read_double("R1 (ohms): ", &R1)) return;

        double frac;
        if (!safe_divide(Vout, (Vin - Vout), &frac)) {
            printf("Error: Vin must not equal Vout (denominator near zero).\n");
            return;
        }
        double R2_ans = R1 * frac;
        printf("R2 = %.6f ohms\n", R2_ans);

        log_printf("Voltage Divider (R2): Vin=%.6f V, Vout=%.6f V, R1=%.6f ohm -> R2=%.6f ohm",
                   Vin, Vout, R1, R2_ans);
    }
    else {
        printf("Invalid selection.\n");
    }
}

// ----------------------- 2) RESISTOR TOOLS (R) --------------------

void menu_item_2(void)
{
    printf("\n--- Resistor Tools ---\n");
    printf("1) Series\n");
    printf("2) Parallel (2 resistors)\n");

    int group;
    if (!read_int("Select: ", &group)) return;

    if (group == 1) {
        printf("\nSeries modes:\n");
        printf("1) Total Rt given n resistors\n");
        printf("2) Missing resistor given Rt and the other (n-1)\n");

        int mode;
        if (!read_int("Select: ", &mode)) return;

        if (mode == 1) {
            // R_total = R1 + R2 + ... + Rn
            int n;
            if (!read_int("How many resistors? ", &n)) return;
            if (n <= 0) { printf("Count must be positive.\n"); return; }

            double sum = 0.0;
            for (int i = 1; i <= n; ++i) {
                double r;
                printf("R%d (ohms): ", i);
                if (!read_double(NULL, &r)) return;
                sum += r;
            }
            printf("R_total(series) = %.6f ohms\n", sum);

            log_printf("Resistors Series: n=%d -> Rt=%.6f ohm", n, sum);
        }
        else if (mode == 2) {
            // R_missing = Rt - sum(known resistors)
            int n;
            if (!read_int("Total number of series resistors n: ", &n)) return;
            if (n < 2) { printf("n must be at least 2.\n"); return; }

            double Rt;
            if (!read_double("Target Rt (ohms): ", &Rt)) return;

            double sum_known = 0.0;
            for (int i = 1; i <= n - 1; ++i) {
                double r;
                printf("Known R%d (ohms): ", i);
                if (!read_double(NULL, &r)) return;
                sum_known += r;
            }

            double missing = Rt - sum_known;
            printf("Missing resistor = %.6f ohms\n", missing);

            log_printf("Resistors Series Missing: n=%d, Rt=%.6f ohm, sum_known=%.6f ohm -> R_missing=%.6f ohm",
                       n, Rt, sum_known, missing);
        }
        else {
            printf("Invalid selection.\n");
        }
    }
    else if (group == 2) {
        printf("\nParallel(2) modes:\n");
        printf("1) Req given R1 and R2\n");
        printf("2) R1  given Req and R2\n");
        printf("3) R2  given Req and R1\n");

        int mode;
        if (!read_int("Select: ", &mode)) return;

        if (mode == 1) {
            // Req = (R1 * R2) / (R1 + R2)
            double R1, R2;
            if (!read_double("R1 (ohms): ", &R1)) return;
            if (!read_double("R2 (ohms): ", &R2)) return;

            // If either branch is a short circuit, equivalent is 0 ohms.
            if (R1 == 0.0 || R2 == 0.0) {
                printf("Req = 0 ohms (one branch is a short).\n");
                log_printf("Resistors Parallel(2): R1=%.6f ohm, R2=%.6f ohm -> Req=0 (short branch)", R1, R2);
                return;
            }

            double Req;
            if (!safe_divide((R1 * R2), (R1 + R2), &Req)) {
                printf("Error: R1 + R2 cannot be zero (or near zero).\n");
                return;
            }
            printf("R_eq(parallel,2) = %.6f ohms\n", Req);

            log_printf("Resistors Parallel(2): R1=%.6f ohm, R2=%.6f ohm -> Req=%.6f ohm", R1, R2, Req);
        }
        else if (mode == 2) {
            // R1 = (Req * R2) / (R2 - Req)
            double Req, R2;
            if (!read_double("Req (ohms): ", &Req)) return;
            if (!read_double("R2  (ohms): ", &R2)) return;

            double R1;
            if (!safe_divide((Req * R2), (R2 - Req), &R1)) {
                printf("Error: R2 must not equal Req (denominator near zero).\n");
                return;
            }
            printf("R1 = %.6f ohms\n", R1);

            log_printf("Resistors Parallel(2) solve R1: Req=%.6f ohm, R2=%.6f ohm -> R1=%.6f ohm", Req, R2, R1);
        }
        else if (mode == 3) {
            // R2 = (Req * R1) / (R1 - Req)
            double Req, R1;
            if (!read_double("Req (ohms): ", &Req)) return;
            if (!read_double("R1  (ohms): ", &R1)) return;

            double R2;
            if (!safe_divide((Req * R1), (R1 - Req), &R2)) {
                printf("Error: R1 must not equal Req (denominator near zero).\n");
                return;
            }
            printf("R2 = %.6f ohms\n", R2);

            log_printf("Resistors Parallel(2) solve R2: Req=%.6f ohm, R1=%.6f ohm -> R2=%.6f ohm", Req, R1, R2);
        }
        else {
            printf("Invalid selection.\n");
        }
    }
    else {
        printf("Invalid selection.\n");
    }
}

// ------------------- 3) AC REACTANCE & RESONANCE ------------------

void menu_item_3(void)
{
    printf("\n--- AC Reactance & Resonance ---\n");
    printf("1) Inductive Reactance (X_L)\n");
    printf("2) Capacitive Reactance (X_C)\n");
    printf("3) Resonance (f0)\n");

    int group;
    if (!read_int("Select: ", &group)) return;

    if (group == 1) {
        printf("\nSolve for:\n");
        printf("1) X_L given f, L\n");
        printf("2) L   given X_L, f\n");
        printf("3) f   given X_L, L\n");

        int mode;
        if (!read_int("Select: ", &mode)) return;

        if (mode == 1) {
            // X_L = 2π f L
            double f, L;
            if (!read_double("f (Hz): ", &f)) return;
            if (!read_double("L (H): ", &L)) return;
            if (f <= 0.0 || L < 0.0) { printf("Error: f>0, L>=0.\n"); return; }

            double XL = 2.0 * PI * f * L;
            printf("X_L = %.6f ohms\n", XL);

            log_printf("AC Inductive Reactance: f=%.6f Hz, L=%.9f H -> XL=%.6f ohm", f, L, XL);
        }
        else if (mode == 2) {
            // L = X_L / (2π f)
            double XL, f;
            if (!read_double("X_L (ohms): ", &XL)) return;
            if (!read_double("f (Hz): ", &f)) return;
            if (f <= 0.0) { printf("Error: f>0.\n"); return; }

            double L;
            if (!safe_divide(XL, (2.0 * PI * f), &L)) { printf("Error: invalid denominator.\n"); return; }
            printf("L = %.9f H\n", L);

            log_printf("AC Inductive Reactance solve L: XL=%.6f ohm, f=%.6f Hz -> L=%.9f H", XL, f, L);
        }
        else if (mode == 3) {
            // f = X_L / (2π L)
            double XL, L;
            if (!read_double("X_L (ohms): ", &XL)) return;
            if (!read_double("L (H): ", &L)) return;
            if (L <= 0.0) { printf("Error: L>0.\n"); return; }

            double f;
            if (!safe_divide(XL, (2.0 * PI * L), &f)) { printf("Error: invalid denominator.\n"); return; }
            printf("f = %.6f Hz\n", f);

            log_printf("AC Inductive Reactance solve f: XL=%.6f ohm, L=%.9f H -> f=%.6f Hz", XL, L, f);
        }
        else {
            printf("Invalid selection.\n");
        }
    }
    else if (group == 2) {
        printf("\nSolve for:\n");
        printf("1) X_C given f, C\n");
        printf("2) C   given X_C, f\n");
        printf("3) f   given X_C, C\n");

        int mode;
        if (!read_int("Select: ", &mode)) return;

        if (mode == 1) {
            // X_C = 1 / (2π f C)
            double f, C;
            if (!read_double("f (Hz): ", &f)) return;
            if (!read_double("C (F): ", &C)) return;
            if (f <= 0.0 || C <= 0.0) { printf("Error: f>0, C>0.\n"); return; }

            double XC;
            if (!safe_divide(1.0, (2.0 * PI * f * C), &XC)) { printf("Error: invalid denominator.\n"); return; }
            printf("X_C = %.6f ohms\n", XC);

            log_printf("AC Capacitive Reactance: f=%.6f Hz, C=%.9e F -> XC=%.6f ohm", f, C, XC);
        }
        else if (mode == 2) {
            // C = 1 / (2π f X_C)
            double XC, f;
            if (!read_double("X_C (ohms): ", &XC)) return;
            if (!read_double("f (Hz): ", &f)) return;
            if (f <= 0.0 || XC <= 0.0) { printf("Error: f>0, X_C>0.\n"); return; }

            double C;
            if (!safe_divide(1.0, (2.0 * PI * f * XC), &C)) { printf("Error: invalid denominator.\n"); return; }
            printf("C = %.9e F\n", C);

            log_printf("AC Capacitive Reactance solve C: XC=%.6f ohm, f=%.6f Hz -> C=%.9e F", XC, f, C);
        }
        else if (mode == 3) {
            // f = 1 / (2π C X_C)
            double XC, C;
            if (!read_double("X_C (ohms): ", &XC)) return;
            if (!read_double("C (F): ", &C)) return;
            if (C <= 0.0 || XC <= 0.0) { printf("Error: C>0, X_C>0.\n"); return; }

            double f;
            if (!safe_divide(1.0, (2.0 * PI * C * XC), &f)) { printf("Error: invalid denominator.\n"); return; }
            printf("f = %.6f Hz\n", f);

            log_printf("AC Capacitive Reactance solve f: XC=%.6f ohm, C=%.9e F -> f=%.6f Hz", XC, C, f);
        }
        else {
            printf("Invalid selection.\n");
        }
    }
    else if (group == 3) {
        printf("\nSolve for:\n");
        printf("1) f0 given L, C\n");
        printf("2) L  given f0, C\n");
        printf("3) C  given f0, L\n");

        int mode;
        if (!read_int("Select: ", &mode)) return;

        if (mode == 1) {
            // f0 = 1 / (2π √(LC))
            double L, C;
            if (!read_double("L (H): ", &L)) return;
            if (!read_double("C (F): ", &C)) return;
            if (L <= 0.0 || C <= 0.0) { printf("Error: L>0, C>0.\n"); return; }

            double f0;
            if (!safe_divide(1.0, (2.0 * PI * sqrt(L * C)), &f0)) { printf("Error: invalid denominator.\n"); return; }
            printf("f0 = %.6f Hz\n", f0);

            log_printf("Resonance: L=%.9e H, C=%.9e F -> f0=%.6f Hz", L, C, f0);
        }
        else if (mode == 2) {
            // L = 1 / ((2π f0)^2 * C)
            double f0, C;
            if (!read_double("f0 (Hz): ", &f0)) return;
            if (!read_double("C (F): ", &C)) return;
            if (f0 <= 0.0 || C <= 0.0) { printf("Error: f0>0, C>0.\n"); return; }

            double denom = (2.0 * PI * f0) * (2.0 * PI * f0) * C;
            double L;
            if (!safe_divide(1.0, denom, &L)) { printf("Error: invalid denominator.\n"); return; }
            printf("L = %.9e H\n", L);

            log_printf("Resonance solve L: f0=%.6f Hz, C=%.9e F -> L=%.9e H", f0, C, L);
        }
        else if (mode == 3) {
            // C = 1 / ((2π f0)^2 * L)
            double f0, L;
            if (!read_double("f0 (Hz): ", &f0)) return;
            if (!read_double("L (H): ", &L)) return;
            if (f0 <= 0.0 || L <= 0.0) { printf("Error: f0>0, L>0.\n"); return; }

            double denom = (2.0 * PI * f0) * (2.0 * PI * f0) * L;
            double C;
            if (!safe_divide(1.0, denom, &C)) { printf("Error: invalid denominator.\n"); return; }
            printf("C = %.9e F\n", C);

            log_printf("Resonance solve C: f0=%.6f Hz, L=%.9e H -> C=%.9e F", f0, L, C);
        }
        else {
            printf("Invalid selection.\n");
        }
    }
    else {
        printf("Invalid selection.\n");
    }
}

// ------------------------ 4) RC TRANSIENT --------------------

void menu_item_4(void)
{
    printf("\n--- RC Transient Calculator ---\n");
    printf("1) Given R, C, t  -> tau, %%charge, %%discharge\n");
    printf("2) Given R, C, %%charge -> t\n");
    printf("3) Given tau, t   -> %%charge, %%discharge\n");
    printf("4) Given R, %%charge, t -> C\n");
    printf("5) Given C, %%charge, t -> R\n");

    int mode;
    if (!read_int("Select: ", &mode)) return;

    if (mode == 1) {
        // tau = R*C, charge% = 100(1-e^-t/tau), discharge% = 100(e^-t/tau)
        double R, C, t;
        if (!read_double("R (ohms): ", &R)) return;
        if (!read_double("C (F): ", &C)) return;
        if (!read_double("t (s): ", &t)) return;

        if (R <= 0.0 || C <= 0.0) { printf("Error: R>0, C>0.\n"); return; }
        if (t < 0.0) { printf("Error: t>=0.\n"); return; }

        double tau = R * C;
        double charge = 100.0 * (1.0 - exp(-t / tau));
        double discharge = 100.0 * exp(-t / tau);

        printf("Tau = %.6f s\n", tau);
        printf("Charge at t: %.2f%%\n", charge);
        printf("Discharge at t: %.2f%%\n", discharge);

        log_printf("RC Transient: R=%.6f ohm, C=%.9e F, t=%.6f s -> tau=%.6f s, charge=%.2f%%, discharge=%.2f%%",
                   R, C, t, tau, charge, discharge);
    }
    else if (mode == 2) {
        // t = -tau * ln(1 - p), where p = charge%/100
        double R, C, pct;
        if (!read_double("R (ohms): ", &R)) return;
        if (!read_double("C (F): ", &C)) return;
        if (!read_double("Target charge (%): ", &pct)) return;

        if (R <= 0.0 || C <= 0.0) { printf("Error: R>0, C>0.\n"); return; }
        if (pct <= 0.0 || pct >= 100.0) { printf("Error: %% must be in (0,100).\n"); return; }

        double tau = R * C;
        double t_ans = -tau * log(1.0 - pct / 100.0);
        printf("t = %.6f s\n", t_ans);

        log_printf("RC solve t: R=%.6f ohm, C=%.9e F, charge=%.2f%% -> t=%.6f s", R, C, pct, t_ans);
    }
    else if (mode == 3) {
        // charge% = 100(1-e^-t/tau), discharge% = 100(e^-t/tau)
        double tau, t;
        if (!read_double("Tau (s): ", &tau)) return;
        if (!read_double("t (s): ", &t)) return;

        if (tau <= 0.0) { printf("Error: tau>0.\n"); return; }
        if (t < 0.0) { printf("Error: t>=0.\n"); return; }

        double charge = 100.0 * (1.0 - exp(-t / tau));
        double discharge = 100.0 * exp(-t / tau);

        printf("Charge at t: %.2f%%\n", charge);
        printf("Discharge at t: %.2f%%\n", discharge);

        log_printf("RC from tau,t: tau=%.6f s, t=%.6f s -> charge=%.2f%%, discharge=%.2f%%",
                   tau, t, charge, discharge);
    }
    else if (mode == 4) {
        // C = tau/R, tau = -t / ln(1 - p)
        double R, pct, t;
        if (!read_double("R (ohms): ", &R)) return;
        if (!read_double("Target charge (%): ", &pct)) return;
        if (!read_double("t (s): ", &t)) return;

        if (R <= 0.0) { printf("Error: R>0.\n"); return; }
        if (t < 0.0) { printf("Error: t>=0.\n"); return; }
        if (pct <= 0.0 || pct >= 100.0) { printf("Error: %% must be in (0,100).\n"); return; }

        double ln_arg = 1.0 - pct / 100.0;
        if (ln_arg <= 0.0) { printf("Error: invalid ln() domain.\n"); return; }

        double tau = -t / log(ln_arg);

        double C;
        if (!safe_divide(tau, R, &C)) { printf("Error: division by zero.\n"); return; }

        printf("C = %.9e F (Tau = %.6f s)\n", C, tau);

        log_printf("RC solve C: R=%.6f ohm, charge=%.2f%%, t=%.6f s -> C=%.9e F (tau=%.6f s)",
                   R, pct, t, C, tau);
    }
    else if (mode == 5) {
        // R = tau/C, tau = -t / ln(1 - p)
        double C, pct, t;
        if (!read_double("C (F): ", &C)) return;
        if (!read_double("Target charge (%): ", &pct)) return;
        if (!read_double("t (s): ", &t)) return;

        if (C <= 0.0) { printf("Error: C>0.\n"); return; }
        if (t < 0.0) { printf("Error: t>=0.\n"); return; }
        if (pct <= 0.0 || pct >= 100.0) { printf("Error: %% must be in (0,100).\n"); return; }

        double ln_arg = 1.0 - pct / 100.0;
        if (ln_arg <= 0.0) { printf("Error: invalid ln() domain.\n"); return; }

        double tau = -t / log(ln_arg);

        double R;
        if (!safe_divide(tau, C, &R)) { printf("Error: division by zero.\n"); return; }

        printf("R = %.6f ohms (Tau = %.6f s)\n", R, tau);

        log_printf("RC solve R: C=%.9e F, charge=%.2f%%, t=%.6f s -> R=%.6f ohm (tau=%.6f s)",
                   C, pct, t, R, tau);
    }
    else {
        printf("Invalid selection.\n");
    }
}

// --------------------5) POWER ------------------------------

void menu_item_5(void)
{
    printf("\n--- Power Equation ---\n");
    printf("Choose using P = V × I:\n");
    printf("1) Power  (P)  given V and I\n");
    printf("2) Voltage (V) given P and I\n");
    printf("3) Current (I) given P and V\n");

    int mode;
    if (!read_int("Select: ", &mode)) return;

    if (mode == 1) {
        // P = V * I
        double V, I;
        if (!read_double("V (volts): ", &V)) return;
        if (!read_double("I (amps):  ", &I)) return;

        double P = V * I;
        printf("P = %.6f W\n", P);

        log_printf("Power: V=%.6f V, I=%.6f A -> P=%.6f W", V, I, P);
    }
    else if (mode == 2) {
        // V = P / I
        double P, I;
        if (!read_double("P (watts): ", &P)) return;
        if (!read_double("I (amps):  ", &I)) return;

        double V;
        if (!safe_divide(P, I, &V)) {
            printf("Error: I cannot be zero (or near zero).\n");
            return;
        }
        printf("V = %.6f V\n", V);

        log_printf("Power solve V: P=%.6f W, I=%.6f A -> V=%.6f V", P, I, V);
    }
    else if (mode == 3) {
        // I = P / V
        double P, V;
        if (!read_double("P (watts): ", &P)) return;
        if (!read_double("V (volts): ", &V)) return;

        double I;
        if (!safe_divide(P, V, &I)) {
            printf("Error: V cannot be zero (or near zero).\n");
            return;
        }
        printf("I = %.6f A\n", I);

        log_printf("Power solve I: P=%.6f W, V=%.6f V -> I=%.6f A", P, V, I);
    }
    else {
        printf("Invalid selection.\n");
    }
}
